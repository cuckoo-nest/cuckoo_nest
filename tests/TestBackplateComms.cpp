#include "Backplate/Message.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::_;

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
}; 

TEST_F(TestBackplateComms, BasicMessageCreation) 
{
    Message msg(MessageCommand::Reset);
    auto rawMessage = msg.GetRawMessage();

    ASSERT_EQ(9, rawMessage.size()); // 3 bytes preamble + 2 bytes command + 2 bytes length + 2 bytes CRC
    EXPECT_THAT(rawMessage, ElementsAre(0xd5, 0x5d, 0xc3, 0xff, 0x00, 0x00, 0x00, _, _)); // last two bytes are CRC
}

// CRC value is populated correctly
TEST_F(TestBackplateComms, CrcCalculation)
{
    Message msg(MessageCommand::Reset);
    auto rawMessage = msg.GetRawMessage();

    EXPECT_THAT(rawMessage[7], 0xA3); 
    EXPECT_THAT(rawMessage[8], 0x4B); 
}

TEST_F(TestBackplateComms, ParseTooShortData)
{
    Message msg;
    uint8_t data[8] = {0};
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_FALSE(result);
}

TEST_F(TestBackplateComms, ParseIncorrectPreamble)
{
    Message msg;
    uint8_t data[10] = {0x00, 0x00, 0x00}; // Incorrect preamble
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_FALSE(result);
}

TEST_F(TestBackplateComms, ParseValidMessage)
{
    Message msg;
    uint8_t data[] = {0xd5, 0x5d, 0xc3, 0xff, 0x00, 0x00,0x00, 0xA3, 0x4B}; // Valid message
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_TRUE(result);
}

TEST_F(TestBackplateComms, ParseInvalidChecksum)
{
    Message msg;
    uint8_t data[10] = {0xd5, 0x5d, 0xc3, 0xff, 0x00, 0x00,0x00, 0x00, 0x00}; // Invalid checksum
    bool result = msg.ParseMessage(data, sizeof(data));
    EXPECT_FALSE(result);
}

// Handle data length over the max size of uint16_t