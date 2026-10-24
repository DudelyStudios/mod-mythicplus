#include "MPModifiers.h"

#include <gtest/gtest.h>

using namespace MythicPlus;

TEST(MPModifierLadderTest, KeyTwoIsFortifiedOnly)
{
    const auto names = ModifiersForKey(2, {});
    ASSERT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "Fortified");
}

TEST(MPModifierLadderTest, KeyTwelveHasAllFive)
{
    const auto names = ModifiersForKey(12, {});
    ASSERT_EQ(names.size(), 5u);
    EXPECT_EQ(names[4], "Savage");
}

TEST(MPModifierLadderTest, BandsStepUp)
{
    EXPECT_EQ(ModifiersForKey(3, {}).size(), 1u);
    EXPECT_EQ(ModifiersForKey(4, {}).size(), 2u);
    EXPECT_EQ(ModifiersForKey(11, {}).size(), 4u);
    EXPECT_EQ(ModifiersForKey(1, {}).size(), 1u);
}

TEST(MPModifierLadderTest, OmittedFortifiedStaysOff)
{
    ModifierBands bands;
    bands.Fortified = 0;
    const auto names = ModifiersForKey(2, bands);
    EXPECT_FALSE(IsFortified(names));
}

TEST(MPModifierLadderTest, FortifiedAndTyrannicalSplitRoles)
{
    const auto names = ModifiersForKey(4, {});
    EXPECT_TRUE(IsFortified(names));
    EXPECT_TRUE(IsTyrannical(names));
}
