#ifndef MP_CONFIG_H
#define MP_CONFIG_H

#include "MPTypes.h"

#include <string>
#include <vector>

namespace MythicPlus
{

class MPConfig
{
public:
    static MPConfig* Instance();

    void LoadFromServer();
    Catalog LoadFromLines(const std::vector<std::string>& dungeonLines,
                          const std::vector<std::string>& themeLines);

    const Catalog& GetCatalog() const { return _catalog; }

private:
    Catalog _catalog;
};

bool ParseDungeonLine(const std::string& line, DungeonDef& out);
bool ParseThemeLine(const std::string& line, uint32_t id, ThemeDef& out);

} // namespace MythicPlus

#endif
