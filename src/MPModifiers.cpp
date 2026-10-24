#include "MPModifiers.h"

namespace MythicPlus
{
namespace
{
void AddAt(std::vector<std::string>& names, uint32_t keyLevel, uint32_t band, const char* name)
{
    if (band > 0 && keyLevel >= band)
        names.emplace_back(name);
}
}

std::vector<std::string> ModifiersForKey(uint32_t keyLevel, const ModifierBands& bands)
{
    if (keyLevel < 2)
        keyLevel = 2;

    std::vector<std::string> names;
    AddAt(names, keyLevel, bands.Fortified, "Fortified");
    AddAt(names, keyLevel, bands.Tyrannical, "Tyrannical");
    AddAt(names, keyLevel, bands.Raging, "Raging");
    AddAt(names, keyLevel, bands.Bolstering, "Bolstering");
    AddAt(names, keyLevel, bands.Savage, "Savage");
    return names;
}

bool IsFortified(const std::vector<std::string>& names)
{
    for (const std::string& name : names)
        if (name == "Fortified")
            return true;
    return false;
}

bool IsTyrannical(const std::vector<std::string>& names)
{
    for (const std::string& name : names)
        if (name == "Tyrannical")
            return true;
    return false;
}

} // namespace MythicPlus
