#include "MPConfig.h"

#include "Config.h"
#include "Log.h"

#include <cstdlib>

namespace
{
std::string Trim(const std::string& value)
{
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return {};
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::string StripQuotes(std::string value)
{
    value = Trim(value);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
        return value.substr(1, value.size() - 2);
    return value;
}
}

namespace MythicPlus
{

MPConfig* MPConfig::Instance()
{
    static MPConfig instance;
    return &instance;
}

bool ParseDungeonLine(const std::string& line, DungeonDef& out)
{
    const std::string text = StripQuotes(line);
    const auto comma = text.find(',');
    if (comma == std::string::npos || comma == 0)
        return false;

    char* end = nullptr;
    const unsigned long mapId = std::strtoul(text.c_str(), &end, 10);
    if (end != text.c_str() + comma || mapId == 0)
        return false;

    const std::string name = Trim(text.substr(comma + 1));
    if (name.empty())
        return false;

    out.MapId = static_cast<uint32_t>(mapId);
    out.Name = name;
    return true;
}

bool ParseThemeLine(const std::string& line, uint32_t id, ThemeDef& out)
{
    const std::string text = StripQuotes(line);
    const auto comma = text.find(',');
    if (comma == std::string::npos || comma == 0)
        return false;

    const std::string name = Trim(text.substr(0, comma));
    const std::string typeText = Trim(text.substr(comma + 1));
    if (name.empty() || typeText.empty())
        return false;

    char* end = nullptr;
    const long type = std::strtol(typeText.c_str(), &end, 10);
    if (end == typeText.c_str() || *end != '\0')
        return false;

    out.Id = id;
    out.Name = name;
    out.CreatureTypes = { static_cast<int32_t>(type) };
    return true;
}

Catalog MPConfig::LoadFromLines(const std::vector<std::string>& dungeonLines,
                                const std::vector<std::string>& themeLines)
{
    Catalog catalog;

    for (const std::string& line : dungeonLines)
    {
        if (Trim(line).empty())
            continue;

        DungeonDef dungeon;
        if (!ParseDungeonLine(line, dungeon))
        {
            LOG_ERROR("module", "MythicPlus: skipped a dungeon row");
            continue;
        }

        bool duplicate = false;
        for (const DungeonDef& existing : catalog.Dungeons)
        {
            if (existing.MapId == dungeon.MapId)
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate)
            catalog.Dungeons.push_back(dungeon);
    }

    uint32_t themeId = 1;
    for (const std::string& line : themeLines)
    {
        if (Trim(line).empty())
            break;

        ThemeDef theme;
        if (!ParseThemeLine(line, themeId, theme))
        {
            LOG_ERROR("module", "MythicPlus: skipped theme row {}", themeId);
            ++themeId;
            continue;
        }

        catalog.Themes.push_back(theme);
        ++themeId;
    }

    _catalog = catalog;
    return catalog;
}

void MPConfig::LoadFromServer()
{
    std::vector<std::string> dungeons;
    std::vector<std::string> themes;

    for (uint32_t i = 1; i <= 64; ++i)
    {
        const std::string line = sConfigMgr->GetOption<std::string>(
            "MythicPlus.Dungeon." + std::to_string(i), "");
        if (line.empty())
            break;
        dungeons.push_back(line);
    }

    for (uint32_t i = 1; i <= 20; ++i)
    {
        const std::string line = sConfigMgr->GetOption<std::string>(
            "MythicPlus.Theme." + std::to_string(i), "");
        if (line.empty())
            break;
        themes.push_back(line);
    }

    LoadFromLines(dungeons, themes);
    LOG_INFO("module", "MythicPlus: loaded {} dungeons and {} themes",
        _catalog.Dungeons.size(), _catalog.Themes.size());
}

} // namespace MythicPlus
