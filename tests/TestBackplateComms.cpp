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

    MockSerialPort mockSerialPort;
    MockDateTimeProvider mockDateTimeProvider;
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

// keep alives are send periodically


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