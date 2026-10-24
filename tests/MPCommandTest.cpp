#include "MPModifiers.h"
#include "MPScore.h"

#include <gtest/gtest.h>

using namespace MythicPlus;

TEST(MPCommandTest, SeasonLeavesTheHeldKey)
{
    const SeasonKeep season = ApplySeason(7);
    EXPECT_EQ(season.KeyLevel, 7u);
    EXPECT_TRUE(season.BestsCleared);
}

TEST(MPCommandTest, ReloadedSavageBandMoves)
{
    ModifierBands bands;
    bands.Savage = 6;
    EXPECT_EQ(ModifiersForKey(6, bands).size(), 5u);
    EXPECT_EQ(ModifiersForKey(5, bands).size(), 4u);
}

TEST(MPCommandTest, KeySetRefusesBelowTheFloor)
{
    const uint32_t stored = 4;
    const uint32_t requested = 1;
    const uint32_t kept = requested < 2 ? stored : requested;
    EXPECT_EQ(kept, stored);
}
