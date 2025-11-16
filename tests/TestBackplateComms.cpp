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



// Handle data length over the max size of uint16_t
// crc calculation is done correctly