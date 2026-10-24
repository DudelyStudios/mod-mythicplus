#include "MPRewards.h"

namespace MythicPlus
{

uint32_t ItemLevelForKey(uint32_t keyLevel, uint32_t baseItemLevel, uint32_t perKey)
{
    if (keyLevel < 2)
        keyLevel = 2;
    return baseItemLevel + perKey * (keyLevel - 2);
}

} // namespace MythicPlus
