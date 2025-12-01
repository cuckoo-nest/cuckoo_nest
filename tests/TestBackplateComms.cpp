#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Backplate/BackplateComms.hpp"
#include "Backplate/CommandMessage.hpp"
#include "Backplate/ResponseMessage.hpp"

using ::testing::Return;
using ::testing::_;
using ::testing::InSequence;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::ByRef;

class MockDateTimeProvider : public IDateTimeProvider {
public:
    MOCK_METHOD(int, gettimeofday, (struct timeval &timeval), (override));
};


class MockSerialPort : public ISerialPort {
public:
    MockSerialPort() : ISerialPort("mock_port") {}
    MOCK_METHOD(bool, Open, (BaudRate baudRate), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(int, Read, (char* buffer, int bufferSize), (override));
    MOCK_METHOD(int, Write, (const std::vector<uint8_t>& data), (override));
    MOCK_METHOD(int, SendBreak, (int durationMs), (override));
    MOCK_METHOD(int, Flush, (), (override));
};

class TestBackplateComms : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
        mockCurrentTimeSec = 0;
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    testing::Action<int(char*, int)> mockReadResponse(const std::vector<uint8_t>& responseBuffer) {
        return testing::Invoke([responseBuffer](char* buffer, int bufferSize) {
            std::memcpy(buffer, responseBuffer.data(), responseBuffer.size());
            return static_cast<int>(responseBuffer.size());
        });
    }

    testing::Action<int(timeval&)> mockGetTimeval(int sec, int usec) {
        return testing::Invoke([sec, usec](timeval& tv) {
            tv.tv_sec = sec;
            tv.tv_usec = usec;
            return 0;
        });
    }

    testing::Action<int(timeval&)> mockGetTimevalSecs() {
        return testing::Invoke([&](timeval& tv) {
            
            tv.tv_sec = mockCurrentTimeSec;
            tv.tv_usec = 0;
            return 0;
        });
    }

    MockSerialPort mockSerialPort;
    MockDateTimeProvider mockDateTimeProvider;
    int mockCurrentTimeSec;
};

TEST_F(TestBackplateComms, InitializeOpensSerialPortCorrectly) 
{
    InSequence s;
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    EXPECT_CALL(mockSerialPort, Open(BaudRate::Baud115200))
        .WillOnce(Return(true));

    EXPECT_CALL(mockSerialPort, Flush()).Times(1);

    EXPECT_CALL(mockSerialPort, SendBreak(_))
        .WillOnce(Return(1)); // Assuming SendBreak returns 1 on success
    
    EXPECT_CALL(mockSerialPort, Flush()).Times(1);

    bool result = comms.InitializeSerial();
    EXPECT_TRUE(result);
}

TEST_F(TestBackplateComms, InitializeFailsIfOpenFails) 
{
    InSequence s;
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    EXPECT_CALL(mockSerialPort, Open(BaudRate::Baud115200))
        .WillOnce(Return(false));

    bool result = comms.Initialize();
    EXPECT_FALSE(result);
}

TEST_F(TestBackplateComms, BustStageWorks) 
{
    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimeval(1000, 0));

    InSequence s;
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    EXPECT_CALL(mockSerialPort, Open(BaudRate::Baud115200))
        .WillRepeatedly(Return(true));

    CommandMessage resetMessage(MessageType::Reset);

    EXPECT_CALL(
        mockSerialPort, 
        Write(testing::ElementsAreArray(resetMessage.GetRawMessage()))
    ).WillOnce(Return(resetMessage.GetRawMessage().size())); // Assuming Write returns number of bytes written

    ResponseMessage burstPacket1FetPresenceMessage(MessageType::FetPresenceData);
    burstPacket1FetPresenceMessage.SetPayload(std::vector<uint8_t>{0x01});
    
    ResponseMessage burstPacket2BrkMessage(MessageType::ResponseAscii);
    burstPacket2BrkMessage.SetPayload(std::vector<uint8_t>{'B', 'R', 'K'});
    

    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(burstPacket1FetPresenceMessage.GetRawMessage()))
        .WillOnce(mockReadResponse(burstPacket2BrkMessage.GetRawMessage()));
    

        // Expect Ack for first burst message
    CommandMessage ackMessage(MessageType::FetPresenceAck);
    ackMessage.SetPayload(std::vector<uint8_t>{0x01});
    EXPECT_CALL(
        mockSerialPort, 
        Write(testing::ElementsAreArray(ackMessage.GetRawMessage()))
    ).WillOnce(Return(ackMessage.GetRawMessage().size()));

    EXPECT_TRUE(comms.DoBurstStage());
}

TEST_F(TestBackplateComms, GetInfoStageWorks)
{
    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimeval(1000, 0));

    InSequence s;
    BackplateComms comms (
        &mockSerialPort,
        &mockDateTimeProvider
    );

    ResponseMessage tfeVersionResponse(MessageType::TfeVersion);
    ResponseMessage tfeBuildInfo(MessageType::TfeBuildInfo);
    ResponseMessage backplaceModelAndBsl(MessageType::BackplateModelAndBslId);

    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(tfeVersionResponse.GetRawMessage()))
        .WillOnce(mockReadResponse(tfeBuildInfo.GetRawMessage()))
        .WillOnce(mockReadResponse(backplaceModelAndBsl.GetRawMessage()));


    EXPECT_TRUE(comms.DoInfoGathering());
}

TEST_F(TestBackplateComms, PeriodicalRequestsWork)
{
    BackplateComms comms (
        &mockSerialPort,
        &mockDateTimeProvider
    );

    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    // keep alive is sent every 15 seconds
    CommandMessage keepAliveMessage(MessageType::PeriodicStatusRequest);
     EXPECT_CALL(
        mockSerialPort, 
        Write(testing::ElementsAreArray(keepAliveMessage.GetRawMessage()))
    )
    .Times(5)
    .WillRepeatedly(Return(keepAliveMessage.GetRawMessage().size())); // Assume Write returns number of bytes written

    // Historical data is requested every 60 seconds
    CommandMessage historicalDataRequest(MessageType::GetHistoricalDataBuffers);
     EXPECT_CALL(
        mockSerialPort, 
        Write(testing::ElementsAreArray(historicalDataRequest.GetRawMessage()))
    )
    .Times(2)
    .WillRepeatedly(Return(historicalDataRequest.GetRawMessage().size())); // Assume Write returns number of bytes written
    
    for (int i = 0; i < 13; ++i) {
        comms.MainTaskBody();
        mockCurrentTimeSec += 5;
    }
}

// will need tests for
// no response received to the read - return false
// no BRK response received - return false
// incorrect ascii message is received - return false
// no fet presence response received - return false
// One status message is sent in bust, followed by BRK
// Two status messages are sent in burst, followed by BRK
// No acks are sent until BRK is received
// Correct number of Acks are sent
// Reads of partial data 

TEST_F(TestBackplateComms, HandlesPartialMessagesAcrossCalls)
{
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    // prepare a response message and split it into two parts
    ResponseMessage sensorMsg(MessageType::TempHumidityData);
    sensorMsg.SetPayload(std::vector<uint8_t>{0x10, 0x20, 0x30, 0x40});
    auto raw = sensorMsg.GetRawMessage();
    size_t split = raw.size() / 2;
    std::vector<uint8_t> part1(raw.begin(), raw.begin() + split);
    std::vector<uint8_t> part2(raw.begin() + split, raw.end());

    // Allow writes (keepalive/historical) without strict checking for this test
    EXPECT_CALL(mockSerialPort, Write(_)).WillRepeatedly(Return(1));

    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(part1))
        .WillOnce(mockReadResponse(part2));

    // Call MainTaskBody twice simulating two scheduling ticks
    comms.MainTaskBody(); // reads part1 -> not enough to parse
    comms.MainTaskBody(); // reads part2 -> should parse and log
}

TEST_F(TestBackplateComms, DoBurstStageHandlesPartialReads)
{
    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimeval(1000, 0));

    InSequence s;
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    EXPECT_CALL(mockSerialPort, Open(BaudRate::Baud115200))
        .WillRepeatedly(Return(true));

    CommandMessage resetMessage(MessageType::Reset);

    EXPECT_CALL(
        mockSerialPort,
        Write(testing::ElementsAreArray(resetMessage.GetRawMessage()))
    ).WillOnce(Return(static_cast<int>(resetMessage.GetRawMessage().size())));

    // Prepare a FetPresenceData message and split it into two parts
    ResponseMessage fetPresence(MessageType::FetPresenceData);
    fetPresence.SetPayload(std::vector<uint8_t>{0x01, 0x02, 0x03});
    auto fetRaw = fetPresence.GetRawMessage();
    size_t splitAt = fetRaw.size() / 2;
    std::vector<uint8_t> fetPart1(fetRaw.begin(), fetRaw.begin() + splitAt);
    std::vector<uint8_t> fetPart2(fetRaw.begin() + splitAt, fetRaw.end());

    ResponseMessage brkMsg(MessageType::ResponseAscii);
    brkMsg.SetPayload(std::vector<uint8_t>{'B','R','K'});

    // Serial Read will return the fet message in two chunks, then BRK
    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(fetPart1))
        .WillOnce(mockReadResponse(fetPart2))
        .WillOnce(mockReadResponse(brkMsg.GetRawMessage()));

    // Expect Ack with the full payload
    CommandMessage ackMessage(MessageType::FetPresenceAck);
    ackMessage.SetPayload(std::vector<uint8_t>{0x01,0x02,0x03});
    EXPECT_CALL(
        mockSerialPort,
        Write(testing::ElementsAreArray(ackMessage.GetRawMessage()))
    ).WillOnce(Return(static_cast<int>(ackMessage.GetRawMessage().size())));

    // The current implementation does not accumulate partial reads in DoBurstStage,
    // so this test is expected to fail until the implementation is updated.
    EXPECT_TRUE(comms.DoBurstStage());
}

TEST_F(TestBackplateComms, DoBurstStageHandlesAllMessagesInOneRead)
{
    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimeval(1000, 0));

    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    CommandMessage resetMessage(MessageType::Reset);

    EXPECT_CALL(
        mockSerialPort,
        Write(testing::ElementsAreArray(resetMessage.GetRawMessage()))
    ).WillOnce(Return(static_cast<int>(resetMessage.GetRawMessage().size())));

    // Prepare a FetPresenceData message
    ResponseMessage fetPresence(MessageType::FetPresenceData);
    fetPresence.SetPayload(std::vector<uint8_t>{0x0A, 0x0B});

    // Prepare a BRK message
    ResponseMessage brkMsg(MessageType::ResponseAscii);
    brkMsg.SetPayload(std::vector<uint8_t>{'B','R','K'});

    // Concatenate both messages into one read payload
    std::vector<uint8_t> combined;
    auto r1 = fetPresence.GetRawMessage();
    auto r2 = brkMsg.GetRawMessage();
    combined.insert(combined.end(), r1.begin(), r1.end());
    combined.insert(combined.end(), r2.begin(), r2.end());

    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(combined))
        .WillRepeatedly(Return(0)); // No more data

    // Expect Ack with the full payload
    CommandMessage ackMessage(MessageType::FetPresenceAck);
    ackMessage.SetPayload(std::vector<uint8_t>{0x0A,0x0B});
    EXPECT_CALL(
        mockSerialPort,
        Write(testing::ElementsAreArray(ackMessage.GetRawMessage()))
    ).WillOnce(Return(static_cast<int>(ackMessage.GetRawMessage().size())));

    EXPECT_TRUE(comms.DoBurstStage());
}

namespace {
    static bool s_tempCalled = false;
    static size_t s_tempLen = 0;
    static bool s_pirCalled = false;
    static size_t s_pirLen = 0;
    static bool s_genericCalled = false;
    static uint16_t s_genericType = 0;
    static size_t s_genericLen = 0;

    void testTempCb(const uint8_t* payload, size_t len)
    {
        s_tempCalled = true;
        s_tempLen = len;
    }

    void testPirCb(const uint8_t* payload, size_t len)
    {
        s_pirCalled = true;
        s_pirLen = len;
    }

    void testGenericCb(uint16_t type, const uint8_t* payload, size_t len)
    {
        s_genericCalled = true;
        s_genericType = type;
        s_genericLen = len;
    }
}

TEST_F(TestBackplateComms, TemperatureCallbackInvoked)
{
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);
    comms.AddTemperatureCallback(testTempCb);
    comms.AddGenericEventCallback(testGenericCb);

    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    ResponseMessage sensorMsg(MessageType::TempHumidityData);
    sensorMsg.SetPayload(std::vector<uint8_t>{0x11, 0x22});

    EXPECT_CALL(mockSerialPort, Write(_)).WillRepeatedly(Return(1));
    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(sensorMsg.GetRawMessage()));

    s_tempCalled = false; s_genericCalled = false; s_genericType = 0; s_tempLen = 0;
    comms.MainTaskBody();

    EXPECT_TRUE(s_tempCalled);
    EXPECT_TRUE(s_genericCalled);
    EXPECT_EQ(s_genericType, static_cast<uint16_t>(MessageType::TempHumidityData));
    EXPECT_EQ(s_tempLen, sensorMsg.GetPayload().size());
}

TEST_F(TestBackplateComms, PIRCallbackInvoked)
{
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);
    comms.AddPIRCallback(testPirCb);
    comms.AddGenericEventCallback(testGenericCb);

    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    ResponseMessage pirMsg(MessageType::PirMotionEvent);
    pirMsg.SetPayload(std::vector<uint8_t>{0x01});

    EXPECT_CALL(mockSerialPort, Write(_)).WillRepeatedly(Return(1));
    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(pirMsg.GetRawMessage()));

    s_pirCalled = false; s_genericCalled = false; s_genericType = 0; s_pirLen = 0;
    comms.MainTaskBody();

    EXPECT_TRUE(s_pirCalled);
    EXPECT_TRUE(s_genericCalled);
    EXPECT_EQ(s_genericType, static_cast<uint16_t>(MessageType::PirMotionEvent));
    EXPECT_EQ(s_pirLen, pirMsg.GetPayload().size());
}

TEST_F(TestBackplateComms, CallbacksNotInvokedOnBadCrc)
{
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);
    comms.AddTemperatureCallback(testTempCb);
    comms.AddPIRCallback(testPirCb);
    comms.AddGenericEventCallback(testGenericCb);


    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    ResponseMessage sensorMsg(MessageType::TempHumidityData);
    sensorMsg.SetPayload(std::vector<uint8_t>{0x11, 0x22});
    auto raw = sensorMsg.GetRawMessage();
    // Corrupt last CRC byte
    std::vector<uint8_t> corrupted(raw.begin(), raw.end());
    if (!corrupted.empty()) corrupted[corrupted.size()-1] ^= 0xFF;

    EXPECT_CALL(mockSerialPort, Write(_)).WillRepeatedly(Return(1));
    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(corrupted));

    s_tempCalled = false; s_pirCalled = false; s_genericCalled = false;
    comms.MainTaskBody();

    EXPECT_FALSE(s_tempCalled);
    EXPECT_FALSE(s_pirCalled);
    EXPECT_FALSE(s_genericCalled);
}

TEST_F(TestBackplateComms, MultipleSubscribersReceiveEvents)
{
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);

    std::vector<int> calls;
    auto cb1 = [&](const uint8_t* p, size_t l) { calls.push_back(1); };
    auto cb2 = [&](const uint8_t* p, size_t l) { calls.push_back(2); };
    auto gcb = [&](uint16_t type, const uint8_t* p, size_t l) { calls.push_back(100 + type); };

    comms.AddTemperatureCallback(cb1);
    comms.AddTemperatureCallback(cb2);
    comms.AddGenericEventCallback(gcb);

    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    ResponseMessage sensorMsg(MessageType::TempHumidityData);
    sensorMsg.SetPayload(std::vector<uint8_t>{0x5});

    EXPECT_CALL(mockSerialPort, Write(_)).WillRepeatedly(Return(1));
    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(sensorMsg.GetRawMessage()));

    comms.MainTaskBody();

    // cb1 then cb2 then generic
    ASSERT_GE(calls.size(), 3);
    EXPECT_EQ(calls[0], 1);
    EXPECT_EQ(calls[1], 2);
    EXPECT_EQ(calls[2], 100 + static_cast<int>(MessageType::TempHumidityData));
}

TEST_F(TestBackplateComms, EventOrderingPreservedForMultipleMessages)
{
    BackplateComms comms(&mockSerialPort, &mockDateTimeProvider);
    std::vector<uint16_t> recvOrder;

    auto gcb = [&](uint16_t type, const uint8_t* p, size_t l) { recvOrder.push_back(type); };
    comms.AddGenericEventCallback(gcb);

    EXPECT_CALL(mockDateTimeProvider, gettimeofday(_))
        .WillRepeatedly(mockGetTimevalSecs());

    ResponseMessage tempMsg(MessageType::TempHumidityData);
    tempMsg.SetPayload(std::vector<uint8_t>{0x1});

    ResponseMessage pirMsg(MessageType::PirMotionEvent);
    pirMsg.SetPayload(std::vector<uint8_t>{0x2});

    // Concatenate two raw messages in one read to simulate back-to-back messages
    std::vector<uint8_t> combined;
    auto r1 = tempMsg.GetRawMessage();
    auto r2 = pirMsg.GetRawMessage();
    combined.insert(combined.end(), r1.begin(), r1.end());
    combined.insert(combined.end(), r2.begin(), r2.end());

    EXPECT_CALL(mockSerialPort, Write(_)).WillRepeatedly(Return(1));
    EXPECT_CALL(mockSerialPort, Read(_,_))
        .WillOnce(mockReadResponse(combined));

    comms.MainTaskBody();

    ASSERT_EQ(recvOrder.size(), 2);
    EXPECT_EQ(recvOrder[0], static_cast<uint16_t>(MessageType::TempHumidityData));
    EXPECT_EQ(recvOrder[1], static_cast<uint16_t>(MessageType::PirMotionEvent));
}