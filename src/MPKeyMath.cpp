#include "MPKeyMath.h"

namespace MythicPlus
{

KeyResolveResult ResolveKey(const KeyResolveInput& input)
{
    KeyResolveResult result;
    const uint32_t floor = input.Floor == 0 ? 2 : input.Floor;
    uint32_t level = input.KeyLevel < floor ? floor : input.KeyLevel;
    const uint32_t elapsed = input.ElapsedSeconds + input.Deaths * input.DeathPenaltySeconds;

    auto drop = [&]()
    {
        result.Finished = false;
        result.Timed = false;
        result.LevelReached = 0;
        result.Change = level <= floor ? 0 : -1;
        result.NewLevel = level <= floor ? floor : level - 1;
    };

    if (input.Spawned == 0 || !input.BossesDead)
    {
        drop();
        return result;
    }

    const uint32_t needed = (input.Spawned * input.SharePercent + 99) / 100;
    if (input.Killed < needed)
    {
        drop();
        return result;
    }

    result.Finished = true;
    if (input.LimitSeconds == 0 || elapsed >= input.LimitSeconds)
    {
        result.Timed = false;
        result.LevelReached = 0;
        result.Change = level <= floor ? 0 : -1;
        result.NewLevel = level <= floor ? floor : level - 1;
        return result;
    }

    const uint32_t remaining = input.LimitSeconds - elapsed;
    const uint32_t percentLeft = (remaining * 100) / input.LimitSeconds;
    uint32_t rise = 1;
    if (percentLeft >= input.UpgradePercent3)
        rise = 3;
    else if (percentLeft >= input.UpgradePercent2)
        rise = 2;

    result.Timed = true;
    result.Change = static_cast<int32_t>(rise);
    result.NewLevel = level + rise;
    result.LevelReached = result.NewLevel;
    return result;
}

uint32_t VaultKeyLevel(bool anyTimed, uint32_t bestTimedLevel, uint32_t heldLevel,
                       uint32_t finishes, uint32_t floor)
{
    if (floor == 0)
        floor = 2;
    if (finishes == 0)
        return floor;
    if (anyTimed)
        return bestTimedLevel < floor ? floor : bestTimedLevel;
    return heldLevel < floor ? floor : heldLevel;
}

uint32_t VaultChoiceCount(uint32_t finishes)
{
    return finishes > 3 ? 3 : finishes;
}

bool GroupMayStart(uint32_t playerCount, uint32_t playersBelow80)
{
    return playerCount == 5 && playersBelow80 == 0;
}

StartGate EvaluateStart(bool partyOk)
{
    StartGate gate;
    gate.Allowed = partyOk;
    gate.CooldownBlocked = false;
    gate.FreshInstance = partyOk;
    return gate;
}

} // namespace MythicPlus
