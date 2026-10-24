#include "MPKeyMath.h"

#include <gtest/gtest.h>

using namespace MythicPlus;

namespace
{
KeyResolveInput Base()
{
    KeyResolveInput input;
    input.KeyLevel = 4;
    input.LimitSeconds = 1800;
    input.BossesDead = true;
    input.Spawned = 10;
    input.Killed = 8;
    return input;
}
}

TEST(MPKeyMathTest, TimedClearRisesByThree)
{
    KeyResolveInput input = Base();
    input.ElapsedSeconds = 1000;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_TRUE(result.Finished);
    EXPECT_TRUE(result.Timed);
    EXPECT_EQ(result.NewLevel, 7u);
    EXPECT_EQ(result.LevelReached, 7u);
}

TEST(MPKeyMathTest, TimedClearRisesByTwo)
{
    KeyResolveInput input = Base();
    input.ElapsedSeconds = 1400;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_EQ(result.NewLevel, 6u);
    EXPECT_TRUE(result.Timed);
}

TEST(MPKeyMathTest, TimedClearRisesByOne)
{
    KeyResolveInput input = Base();
    input.ElapsedSeconds = 1700;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_EQ(result.NewLevel, 5u);
}

TEST(MPKeyMathTest, LateClearDropsAndStillFinishes)
{
    KeyResolveInput input = Base();
    input.ElapsedSeconds = 1800;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_TRUE(result.Finished);
    EXPECT_FALSE(result.Timed);
    EXPECT_EQ(result.NewLevel, 3u);
    EXPECT_EQ(result.LevelReached, 0u);
}

TEST(MPKeyMathTest, LivingBossIsAMiss)
{
    KeyResolveInput input = Base();
    input.BossesDead = false;
    input.ElapsedSeconds = 100;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_FALSE(result.Finished);
    EXPECT_EQ(result.NewLevel, 3u);
}

TEST(MPKeyMathTest, FloorStaysAtTwo)
{
    KeyResolveInput input = Base();
    input.KeyLevel = 2;
    input.BossesDead = false;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_EQ(result.NewLevel, 2u);
    EXPECT_EQ(result.Change, 0);
}

TEST(MPKeyMathTest, BestTimedLevelSurvivesALaterDrop)
{
    EXPECT_EQ(VaultKeyLevel(true, 8, 6, 2, 2), 8u);
}

TEST(MPKeyMathTest, DeathPushesAClearOverTheTimer)
{
    KeyResolveInput input = Base();
    input.ElapsedSeconds = 1798;
    input.Deaths = 1;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_TRUE(result.Finished);
    EXPECT_FALSE(result.Timed);
    EXPECT_EQ(result.NewLevel, 3u);
}

TEST(MPKeyMathTest, ZeroSpawnsAreAMiss)
{
    KeyResolveInput input = Base();
    input.Spawned = 0;
    input.Killed = 0;
    input.ElapsedSeconds = 10;
    const KeyResolveResult result = ResolveKey(input);
    EXPECT_FALSE(result.Finished);
    EXPECT_EQ(result.NewLevel, 3u);
}

TEST(MPKeyMathTest, EmptyWeekReturnsTheFloor)
{
    EXPECT_EQ(VaultKeyLevel(false, 0, 3, 0, 2), 2u);
}

TEST(MPKeyMathTest, LateOnlyVaultUsesHeldLevel)
{
    EXPECT_EQ(VaultKeyLevel(false, 0, 3, 1, 2), 3u);
}

TEST(MPKeyMathTest, VaultChoicesCapAtThree)
{
    EXPECT_EQ(VaultChoiceCount(1), 1u);
    EXPECT_EQ(VaultChoiceCount(2), 2u);
    EXPECT_EQ(VaultChoiceCount(3), 3u);
    EXPECT_EQ(VaultChoiceCount(4), 3u);
}
