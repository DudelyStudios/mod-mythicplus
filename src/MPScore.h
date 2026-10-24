#ifndef MP_SCORE_H
#define MP_SCORE_H

#include <cstdint>
#include <vector>

namespace MythicPlus
{

uint32_t MythicScore(const std::vector<uint32_t>& bestTimedLevels, uint32_t perKey = 10);

uint32_t RaiseBest(uint32_t storedBest, uint32_t timedLevelReached);

struct SeasonKeep
{
    uint32_t KeyLevel = 2;
    bool BestsCleared = false;
};

SeasonKeep ApplySeason(uint32_t keyLevel);

} // namespace MythicPlus

#endif
