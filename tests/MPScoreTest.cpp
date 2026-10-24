#include "MPScore.h"

#include <gtest/gtest.h>

using namespace MythicPlus;

TEST(MPScoreTest, SlowerRunDoesNotReplaceTheBest)
{
    EXPECT_EQ(RaiseBest(8, 6), 8u);
    EXPECT_EQ(MythicScore({ 8 }), 80u);
}

TEST(MPScoreTest, TwoDungeonsAdd)
{
    EXPECT_EQ(MythicScore({ 10, 4 }), 140u);
}

TEST(MPScoreTest, SeasonClearsBestsAndKeepsTheKey)
{
    const SeasonKeep season = ApplySeason(7);
    EXPECT_TRUE(season.BestsCleared);
    EXPECT_EQ(season.KeyLevel, 7u);
}

TEST(MPScoreTest, SeasonStartFromEmptyUsesTheNextTimedLevel)
{
    EXPECT_EQ(RaiseBest(0, 4), 4u);
}
