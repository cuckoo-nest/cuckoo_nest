#include <gtest/gtest.h>
#include "Backplate/MessageParser.hpp"
#include "Backplate/ResponseMessage.hpp"
#include "Backplate/Message.hpp"

using namespace std;

TEST(TestMessageParser, ParseValidMessage)
{
    ResponseMessage msg(MessageType::ResponseAscii);
    msg.SetPayload(vector<uint8_t>{'B','R','K'});
    auto raw = msg.GetRawMessage();

    MessageParser parser;
    auto out = parser.Feed(raw.data(), raw.size());

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].GetMessageCommand(), MessageType::ResponseAscii);
    EXPECT_EQ(out[0].GetPayload(), msg.GetPayload());
}

TEST(TestMessageParser, ParseMessageWithTrailingBytes)
{
    ResponseMessage msg(MessageType::ResponseAscii);
    msg.SetPayload(vector<uint8_t>{'B','R','K'});
    auto raw = msg.GetRawMessage();

    vector<uint8_t> buffer(raw.begin(), raw.end());
    buffer.push_back(0x99);
    buffer.push_back(0x88);

    MessageParser parser;
    auto out = parser.Feed(buffer.data(), buffer.size());

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].GetMessageCommand(), MessageType::ResponseAscii);
}

TEST(TestMessageParser, HandlesPartialMessagesAcrossFeeds)
{
    ResponseMessage msg(MessageType::TempHumidityData);
    msg.SetPayload(vector<uint8_t>{0x10,0x20,0x30,0x40});
    auto raw = msg.GetRawMessage();
    size_t split = raw.size() / 2;

    MessageParser parser;
    auto out1 = parser.Feed(raw.data(), split);
    EXPECT_EQ(out1.size(), 0);

    auto out2 = parser.Feed(raw.data() + split, raw.size() - split);
    ASSERT_EQ(out2.size(), 1);
    EXPECT_EQ(out2[0].GetMessageCommand(), MessageType::TempHumidityData);
}

TEST(TestMessageParser, HandlesMultipleMessagesInOneFeed)
{
    ResponseMessage a(MessageType::TempHumidityData);
    a.SetPayload(vector<uint8_t>{0x01});
    ResponseMessage b(MessageType::PirMotionEvent);
    b.SetPayload(vector<uint8_t>{0x02});

    vector<uint8_t> combined;
    auto r1 = a.GetRawMessage();
    auto r2 = b.GetRawMessage();
    combined.insert(combined.end(), r1.begin(), r1.end());
    combined.insert(combined.end(), r2.begin(), r2.end());

    MessageParser parser;
    auto out = parser.Feed(combined.data(), combined.size());

    ASSERT_EQ(out.size(), 2);
    EXPECT_EQ(out[0].GetMessageCommand(), MessageType::TempHumidityData);
    EXPECT_EQ(out[1].GetMessageCommand(), MessageType::PirMotionEvent);
}

TEST(TestMessageParser, BadCrcDoesNotProduceMessageButRecovers)
{
    ResponseMessage good(MessageType::TempHumidityData);
    good.SetPayload(vector<uint8_t>{0x11,0x22});
    auto goodRaw = good.GetRawMessage();

    // Create corrupted copy
    vector<uint8_t> bad(goodRaw.begin(), goodRaw.end());
    if (!bad.empty()) 
    {
        bad[bad.size()-1] ^= 0xFF; // flip final CRC byte
    }

    MessageParser parser;
    auto outBad = parser.Feed(bad.data(), bad.size());
    EXPECT_EQ(outBad.size(), 0);

    auto outGood = parser.Feed(goodRaw.data(), goodRaw.size());
    ASSERT_EQ(outGood.size(), 1);
    EXPECT_EQ(outGood[0].GetMessageCommand(), MessageType::TempHumidityData);
}

TEST(TestMessageParser, ParsesRealWorldExampleStream)
{
    // Example stream provided by user
    uint8_t example[] = {0,0,0,0xd5,0xd5,0xaa,0x96,0x01,0x00,0x1c,0x00,0x31,0x2e,0x30,0x2e,0x32,0x36,0x20,0x32,0x30,0x31,0x39,0x2d,0x30,0x34,0x2d,0x30,0x35,0x20,0x31,0x39,0x3a,0x32,0x34,0x3a,0x33,0x34,0x20,0x4b,0x2b,0x71,0xd5,0xd5,0xaa,0x96,0x04,0x00,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0x51};
    size_t len = sizeof(example) / sizeof(example[0]);

    MessageParser parser;
    auto out = parser.Feed(example, len);

    // Expect at least two messages (ResponseAscii and FetPresenceData)
    ASSERT_GE(out.size(), 2);
    EXPECT_EQ(out[0].GetMessageCommand(), MessageType::ResponseAscii);
    EXPECT_EQ(out[1].GetMessageCommand(), MessageType::FetPresenceData);
}
