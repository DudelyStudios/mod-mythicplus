#ifndef MP_MODIFIERS_H
#define MP_MODIFIERS_H

#include <cstdint>
#include <string>
#include <vector>

namespace MythicPlus
{

struct ModifierBands
{
    uint32_t Fortified = 2;
    uint32_t Tyrannical = 4;
    uint32_t Raging = 7;
    uint32_t Bolstering = 10;
    uint32_t Savage = 12;
};

std::vector<std::string> ModifiersForKey(uint32_t keyLevel, const ModifierBands& bands);

bool IsFortified(const std::vector<std::string>& names);
bool IsTyrannical(const std::vector<std::string>& names);

} // namespace MythicPlus

#endif
