#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Backplate/CommandMessage.hpp"
#include "Backplate/ResponseMessage.hpp"

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::_;

class TestBackplateCommsMessage : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }
}; 

TEST_F(TestBackplateCommsMessage, BasicMessageCreation) 
{
    CommandMessage msg(MessageType::Reset);
    auto rawMessage = msg.GetRawMessage();

    ASSERT_EQ(9, rawMessage.size()); // 3 bytes preamble + 2 bytes command + 2 bytes length + 2 bytes CRC
    EXPECT_THAT(rawMessage, ElementsAre(0xd5, 0xaa, 0x96, 0xff, 0x00, 0x00, 0x00, _, _)); // last two bytes are CRC
}

// CRC value is populated correctly
TEST_F(TestBackplateCommsMessage, CrcCalculation)
{
    CommandMessage msg(MessageType::Reset);
    auto rawMessage = msg.GetRawMessage();

    EXPECT_THAT(rawMessage[7], 0xA3); 
    EXPECT_THAT(rawMessage[8], 0x4B); 
}

TEST_F(TestBackplateCommsMessage, ParseTooShortData)
{
    ResponseMessage msg;
    uint8_t data[8] = {0};
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_FALSE(result);
}

TEST_F(TestBackplateCommsMessage, ParseIncorrectPreamble)
{
    ResponseMessage msg;
    uint8_t data[10] = {0x00, 0x00, 0x00, 0x00}; // Incorrect preamble
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_FALSE(result);
}


// Example BRK:
// preamble d5 d5 aa 96
// command = 01 00
// len = 03 00
// Data = "BRK" 42 52 4B
TEST_F(TestBackplateCommsMessage, ParseValidMessage)
{
    ResponseMessage msg;
    uint8_t data[] = {0xd5, 0xd5, 0xaa, 0x96, 0x01, 0x00, 0x03, 0x00, 0x42, 0x52, 0x4B, 0x0C, 0xB4}; // Valid message
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_TRUE(result);
    EXPECT_EQ(msg.GetMessageCommand(), MessageType::ResponseAscii);
}

TEST_F(TestBackplateCommsMessage, ParseInvalidChecksum)
{
    ResponseMessage msg;
    uint8_t data[11] = {0xd5, 0xd5, 0xaa, 0x96, 0xff, 0x00, 0x00,0x00, 0x00, 0x00}; // Invalid checksum
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_FALSE(result);
}

TEST_F(TestBackplateCommsMessage, CreateAsciiResponseMessage)
{
    ResponseMessage msg(MessageType::ResponseAscii);
    msg.SetPayload(std::vector<uint8_t>{'B', 'R', 'K'});
    auto rawMessage = msg.GetRawMessage(); 

    EXPECT_EQ(13, rawMessage.size()); // 4 bytes preamble + 2 bytes command + 2 bytes length + 3 bytes data + 2 bytes CRC
    EXPECT_THAT(rawMessage, ElementsAre(0xd5, 0xd5, 0xaa, 0x96, 0x01, 0x00, 0x03, 0x00, 0x42, 0x52, 0x4B, _, _));
}

// Parsing of ascii response message
TEST_F(TestBackplateCommsMessage, ParseAsciiResponseMessage)
{
    ResponseMessage msg(MessageType::ResponseAscii);
    msg.SetPayload(std::vector<uint8_t>{'B', 'R', 'K'});
    auto rawMessage = msg.GetRawMessage();

    ResponseMessage parsedMsg;
    bool result = parsedMsg.ParseMessage(rawMessage.data(), rawMessage.size());
    EXPECT_TRUE(result);
    EXPECT_EQ(parsedMsg.GetMessageCommand(), MessageType::ResponseAscii);
    EXPECT_THAT(parsedMsg.GetPayload(), ElementsAre('B', 'R', 'K'));
}

// Handle data length over the max size of uint16_t