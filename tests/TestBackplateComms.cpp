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

    MockSerialPort mockSerialPort;
};

TEST_F(TestBackplateComms, InitializeOpensSerialPortCorrectly) 
{
    InSequence s;
    BackplateComms comms(&mockSerialPort, 1000);

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
    BackplateComms comms(&mockSerialPort, 1000);

    EXPECT_CALL(mockSerialPort, Open(BaudRate::Baud115200))
        .WillOnce(Return(false));

    bool result = comms.Initialize();
    EXPECT_FALSE(result);
}

TEST_F(TestBackplateComms, InitalizeBurstStages) 
{
    InSequence s;
    //BackplateComms comms(&mockSerialPort, 1000);
    BackplateComms comms(&mockSerialPort, 10* 10000 * 1000);

    EXPECT_CALL(mockSerialPort, Open(BaudRate::Baud115200))
        .WillRepeatedly(Return(true));

    CommandMessage resetMessage(MessageType::Reset);

    EXPECT_CALL(
        mockSerialPort, 
        Write(testing::ElementsAreArray(resetMessage.GetRawMessage()))
    ).WillOnce(Return(resetMessage.GetRawMessage().size())); // Assuming Write returns number of bytes written

    ResponseMessage burstPacket1FetPresenceMessage(MessageType::FetPresenceData);
    burstPacket1FetPresenceMessage.SetPayload(std::vector<uint8_t>{0x01});
    std::vector<uint8_t> responsebuffer1 = burstPacket1FetPresenceMessage.GetRawMessage();
    
    ResponseMessage burstPacket2BrkMessage(MessageType::ResponseAscii);
    burstPacket2BrkMessage.SetPayload(std::vector<uint8_t>{'B', 'R', 'K'});
    std::vector<uint8_t> responsebuffer2 = burstPacket2BrkMessage.GetRawMessage();
    
    
    EXPECT_CALL(
        mockSerialPort,
        Read(_,_)
    ).WillOnce(testing::Invoke([responsebuffer1](char* buffer, int bufferSize) {
        std::memcpy(buffer, responsebuffer1.data(), responsebuffer1.size());
        return static_cast<int>(responsebuffer1.size());
    })).WillOnce(testing::Invoke([responsebuffer2](char* buffer, int bufferSize) {
        std::memcpy(buffer, responsebuffer2.data(), responsebuffer2.size());
        return static_cast<int>(responsebuffer2.size());
    }));
    
    
    EXPECT_TRUE(comms.Initialize());
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