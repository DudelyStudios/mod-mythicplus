#include "MPRewards.h"

#include <gtest/gtest.h>

using namespace MythicPlus;

TEST(MPVaultTest, GearStepsFromTheStartingKey)
{
    EXPECT_EQ(ItemLevelForKey(2), 245u);
    EXPECT_EQ(ItemLevelForKey(3), 248u);
    EXPECT_EQ(ItemLevelForKey(4), 251u);
    EXPECT_EQ(ItemLevelForKey(5), 254u);
}

TEST(MPVaultTest, KeyBelowTheFloorUsesKeyTwo)
{
    EXPECT_EQ(ItemLevelForKey(1), 245u);
}
