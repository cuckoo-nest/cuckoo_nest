#include "Backplate/CRC-CITT.hpp"
#include <gtest/gtest.h>

class TestCRCCITT : public ::testing::Test {
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

TEST_F(TestCRCCITT, BasicCRCCalculation) 
{
    CRC_CITT crcCalculator;
    uint8_t data[] = {0x82, 0x00, 0x02, 0x00, 0x00, 0x00};
    uint16_t crc = crcCalculator.Calculate(data, sizeof(data));
    EXPECT_EQ(0xb208, crc);
}