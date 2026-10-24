#ifndef MP_TYPES_H
#define MP_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace MythicPlus
{

struct ThemeDef
{
    uint32_t Id = 0;
    std::string Name;
    std::vector<int32_t> CreatureTypes;
};

struct DungeonDef
{
    uint32_t MapId = 0;
    std::string Name;
};

struct Catalog
{
    std::vector<DungeonDef> Dungeons;
    std::vector<ThemeDef> Themes;
};

} // namespace MythicPlus

#endif
