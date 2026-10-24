#ifndef MP_KEY_MATH_H
#define MP_KEY_MATH_H

#include <cstdint>

namespace MythicPlus
{

struct KeyResolveInput
{
    uint32_t KeyLevel = 2;
    uint32_t LimitSeconds = 1800;
    uint32_t ElapsedSeconds = 0;
    uint32_t Deaths = 0;
    uint32_t DeathPenaltySeconds = 5;
    bool BossesDead = false;
    uint32_t Spawned = 0;
    uint32_t Killed = 0;
    uint32_t SharePercent = 80;
    uint32_t Floor = 2;
    uint32_t UpgradePercent3 = 40;
    uint32_t UpgradePercent2 = 20;
};

struct KeyResolveResult
{
    uint32_t NewLevel = 2;
    bool Finished = false;
    bool Timed = false;
    uint32_t LevelReached = 0;
    int32_t Change = 0;
};

KeyResolveResult ResolveKey(const KeyResolveInput& input);

uint32_t VaultKeyLevel(bool anyTimed, uint32_t bestTimedLevel, uint32_t heldLevel,
                       uint32_t finishes, uint32_t floor);

uint32_t VaultChoiceCount(uint32_t finishes);

bool GroupMayStart(uint32_t playerCount, uint32_t playersBelow80);

struct StartGate
{
    bool Allowed = false;
    bool CooldownBlocked = false;
    bool FreshInstance = false;
};

StartGate EvaluateStart(bool partyOk);

} // namespace MythicPlus

#endif
