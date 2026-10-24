#include "MPBossSpawnQuery.h"
#include "MPConfig.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace MythicPlus;

namespace
{
std::vector<std::string> FirstSeasonDungeons()
{
    return {
        "389,Ragefire Chasm", "36,Deadmines", "33,Shadowfang Keep", "34,The Stockade",
        "43,Wailing Caverns", "48,Blackfathom Deeps", "47,Razorfen Kraul", "90,Gnomeregan",
        "129,Razorfen Downs", "189,Scarlet Monastery", "70,Uldaman", "209,Zul'Farrak",
        "349,Maraudon", "109,Sunken Temple", "230,Blackrock Depths", "229,Blackrock Spire",
        "289,Scholomance", "329,Stratholme", "543,Hellfire Ramparts", "542,Blood Furnace",
        "547,Slave Pens", "546,Underbog", "557,Mana-Tombs", "558,Auchenai Crypts",
        "556,Sethekk Halls", "555,Shadow Labyrinth", "540,Shattered Halls", "553,Botanica",
        "554,Mechanar", "552,Arcatraz", "574,Utgarde Keep", "576,The Nexus",
        "601,Azjol-Nerub", "619,Ahn'kahet", "600,Drak'Tharon Keep", "608,Violet Hold",
        "604,Gundrak", "599,Halls of Stone", "602,Halls of Lightning", "578,The Oculus",
        "575,Utgarde Pinnacle", "595,Culling of Stratholme", "632,Forge of Souls",
        "658,Pit of Saron", "668,Halls of Reflection"
    };
}

std::vector<std::string> FirstSeasonThemes()
{
    return {
        "Beast Hunt,1", "Dragon's Lair,2", "Demonic Invasion,3", "Elemental Chaos,4",
        "Giant's Keep,5", "Undead Rising,6", "Humanoid Stronghold,7",
        "Mechanical Mayhem,9", "Random Chaos,-1"
    };
}

bool HasMap(const Catalog& catalog, uint32_t mapId)
{
    for (const DungeonDef& dungeon : catalog.Dungeons)
        if (dungeon.MapId == mapId)
            return true;
    return false;
}
}

TEST(MPCatalogTest, LoadsFortyFiveMapsAndNineThemes)
{
    MPConfig config;
    const Catalog catalog = config.LoadFromLines(FirstSeasonDungeons(), FirstSeasonThemes());

    EXPECT_EQ(catalog.Dungeons.size(), 45u);
    EXPECT_EQ(catalog.Themes.size(), 9u);
    EXPECT_EQ(HasMap(catalog, 189), true);
    EXPECT_EQ(HasMap(catalog, 36), true);
    EXPECT_EQ(catalog.Themes.back().CreatureTypes.at(0), -1);
}

TEST(MPCatalogTest, RemovingOneMapLeavesTheRest)
{
    auto lines = FirstSeasonDungeons();
    lines.erase(lines.begin() + 1);

    MPConfig config;
    const Catalog catalog = config.LoadFromLines(lines, FirstSeasonThemes());

    EXPECT_EQ(catalog.Dungeons.size(), 44u);
    EXPECT_EQ(HasMap(catalog, 36), false);
    EXPECT_EQ(HasMap(catalog, 389), true);
}

TEST(MPCatalogTest, EmptyCreatureTypeSkipsThatTheme)
{
    const std::vector<std::string> themes = { "Beast Hunt,1", "Broken,", "Undead Rising,6" };

    MPConfig config;
    const Catalog catalog = config.LoadFromLines(FirstSeasonDungeons(), themes);

    ASSERT_EQ(catalog.Themes.size(), 2u);
    EXPECT_EQ(catalog.Themes[0].Name, "Beast Hunt");
    EXPECT_EQ(catalog.Themes[1].Name, "Undead Rising");
}

TEST(MPCatalogTest, BossQueryUsesCurrentImmunitySchema)
{
    const std::string query = BuildBossSpawnPointQuery(229);

    EXPECT_NE(query.find("ct.entry"), std::string::npos);
    EXPECT_NE(query.find("creature_immunities"), std::string::npos);
    EXPECT_NE(query.find("CreatureImmunitiesId"), std::string::npos);
    EXPECT_NE(query.find("MechanicsMask"), std::string::npos);
    EXPECT_EQ(query.find("mechanic_immune_mask"), std::string::npos);
    EXPECT_NE(query.find("c.map = 229"), std::string::npos);
    EXPECT_EQ(query.find("ct.name"), std::string::npos);
}
