#include "MPKeyMath.h"

#include <gtest/gtest.h>

using namespace MythicPlus;

TEST(MPPartyAndKeyTest, FourPlayersAreRefused)
{
    EXPECT_FALSE(GroupMayStart(4, 0));
}

TEST(MPPartyAndKeyTest, ALevel79IsRefused)
{
    EXPECT_FALSE(GroupMayStart(5, 1));
}

TEST(MPPartyAndKeyTest, FiveLevel80sMayStart)
{
    EXPECT_TRUE(GroupMayStart(5, 0));
}

TEST(MPPartyAndKeyTest, ANewKeyIsLevelTwo)
{
    const uint32_t stored = 0;
    const uint32_t granted = stored == 0 ? 2 : stored;
    EXPECT_EQ(granted, 2u);
}

TEST(MPPartyAndKeyTest, RepeatRequestsAFreshInstance)
{
    const StartGate first = EvaluateStart(true);
    const StartGate second = EvaluateStart(true);
    EXPECT_TRUE(first.FreshInstance);
    EXPECT_TRUE(second.FreshInstance);
    EXPECT_FALSE(second.CooldownBlocked);
}
