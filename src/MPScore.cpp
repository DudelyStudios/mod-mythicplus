#include "MPScore.h"

namespace MythicPlus
{

uint32_t MythicScore(const std::vector<uint32_t>& bestTimedLevels, uint32_t perKey)
{
    uint32_t total = 0;
    for (uint32_t best : bestTimedLevels)
        total += best * perKey;
    return total;
}

uint32_t RaiseBest(uint32_t storedBest, uint32_t timedLevelReached)
{
    return timedLevelReached > storedBest ? timedLevelReached : storedBest;
}

SeasonKeep ApplySeason(uint32_t keyLevel)
{
    SeasonKeep result;
    result.KeyLevel = keyLevel;
    result.BestsCleared = true;
    return result;
}

} // namespace MythicPlus
