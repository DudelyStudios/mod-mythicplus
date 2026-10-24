#ifndef MYTHIC_PLUS_MGR_H
#define MYTHIC_PLUS_MGR_H

#include "MPTypes.h"
#include "ObjectGuid.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Player;
class Creature;
class Group;
class InstanceMap;
class Map;

namespace MythicPlus
{

struct RunPlayer
{
    ObjectGuid Guid;
    uint32_t ReturnMap = 0;
    float ReturnX = 0;
    float ReturnY = 0;
    float ReturnZ = 0;
    float ReturnO = 0;
};

struct SpawnedMob
{
    ObjectGuid Guid;
    bool IsBoss = false;
    bool Credited = false;
};

struct Run
{
    uint32_t Id = 0;
    ObjectGuid KeyHolder;
    uint32_t KeyLevel = 2;
    uint32_t ThemeId = 0;
    uint32_t MapId = 0;
    uint32_t InstanceId = 0;
    uint32_t WeekId = 0;
    uint64_t StartedAt = 0;
    bool Populated = false;
    bool Resolved = false;
    bool Arrived = false;
    bool Abandoned = false;
    bool ResetPending = false;
    uint32_t Spawned = 0;
    uint32_t Killed = 0;
    uint32_t Bosses = 0;
    uint32_t BossesKilled = 0;
    uint32_t Deaths = 0;
    float EntranceX = 0;
    float EntranceY = 0;
    float EntranceZ = 0;
    float EntranceO = 0;
    std::vector<RunPlayer> Players;
    std::vector<SpawnedMob> Mobs;
    std::string ModifierText;
};

struct KeyRecord
{
    uint32_t Level = 2;
    uint32_t WeekId = 0;
    uint32_t Finishes = 0;
    uint32_t BestTimed = 0;
    uint32_t VaultLevel = 0;
    bool AnyTimed = false;
    bool Exists = false;
};

class MythicPlusMgr
{
public:
    static MythicPlusMgr* Instance();

    void Initialize();
    void Update(uint32 diff);

    bool TryStart(Player* confirmer, uint32_t themeId, uint32_t mapId, std::string& error);
    void TryPopulate(Player* player);
    void NoteDeath(Player* player);
    void Abort(Player* player);
    Run* GetRunByPlayer(ObjectGuid guid);
    KeyRecord LoadKey(uint32_t guidLow);
    void EnsureCurrentWeek(uint32_t guidLow);
    void SaveKey(uint32_t guidLow, const KeyRecord& key);
    bool SetHeldKey(uint32_t guidLow, uint32_t level);
    uint32_t CurrentWeekId() const;
    uint32_t SeasonId() const { return _seasonId; }
    void StartSeason();
    std::string ScoreText(uint32_t guidLow) const;
    std::string BoardText(uint32_t mapId) const;
    std::string VaultText(uint32_t guidLow) const;
    bool ClaimVaultChoice(Player* player, uint32_t index);
    void SendRunState(const Run& run);
    void NoteCreatureLevel(ObjectGuid guid, uint8 level, uint32 health, uint32 instanceId, float damageMult);
    bool OwnedLevel(ObjectGuid guid, uint8& level, uint32& health) const;
    float OwnedDamage(ObjectGuid guid) const;
    void ForgetCreature(ObjectGuid guid);
    bool HasOwned() const { return !_owned.empty(); }
    void StripEquipableLoot(Creature* creature) const;
    uint32_t ActiveModifierCount(uint32_t keyLevel) const;

private:
    struct Owned
    {
        uint8 Level = 0;
        uint32 Health = 0;
        uint32 InstanceId = 0;
        float Damage = 1.0f;
    };

    struct PoolEntry
    {
        uint32 Entry = 0;
        uint32 Type = 0;
    };

    void LoadPools();
    void LoadStats();
    bool Populate(Run& run, InstanceMap* map);
    void Resolve(Run& run);
    void ApplyResetIfDue(uint32_t guidLow, KeyRecord& key, bool runStillOpen);
    void GiveGear(Player* player, uint32_t itemLevel, const char* subject);
    void RememberBest(uint32_t guidLow, uint32_t mapId, uint32_t levelReached, uint32_t clearSeconds, uint32_t themeId, uint32_t keyLevel, const std::string& name);
    uint32_t PickEntry(int32_t creatureType, bool boss) const;
    void Broadcast(Player* player, const std::string& payload) const;

    std::unordered_map<uint32_t, Run> _runs;
    std::unordered_map<uint64_t, uint32_t> _playerRuns;
    std::unordered_map<uint32_t, std::vector<PoolEntry>> _trash;
    std::unordered_map<uint32_t, std::vector<PoolEntry>> _bosses;
    std::unordered_map<ObjectGuid, Owned> _owned;
    std::unordered_map<uint32_t, std::vector<ObjectGuid>> _instanceGuids;
    uint32_t _nextRunId = 1;
    uint32_t _seasonId = 1;
    uint32_t _updateTimer = 0;
};

} // namespace MythicPlus

#define sMythicPlus MythicPlus::MythicPlusMgr::Instance()

#endif
