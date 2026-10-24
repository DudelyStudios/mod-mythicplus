#include "MythicPlusMgr.h"

#include "MPBossSpawnQuery.h"
#include "MPConfig.h"
#include "MPKeyMath.h"
#include "MPModifiers.h"
#include "MPRewards.h"
#include "MPScore.h"

#include "Chat.h"
#include "Common.h"
#include "Config.h"
#include "DBCEnums.h"
#include "Creature.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "GameTime.h"
#include "Group.h"
#include "InstanceSaveMgr.h"
#include "InstanceScript.h"
#include "Item.h"
#include "Log.h"
#include "LootMgr.h"
#include "Mail.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Random.h"
#include "SharedDefines.h"
#include "WorldSession.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sstream>

namespace MythicPlus
{
namespace
{
uint32 ConfU(const char* key, uint32 fallback)
{
    return sConfigMgr->GetOption<uint32>(key, fallback);
}

std::string ConfS(const char* key, const std::string& fallback)
{
    return sConfigMgr->GetOption<std::string>(key, fallback);
}

ModifierBands LoadBands()
{
    ModifierBands bands;
    auto band = [](const char* key, uint32 fallback)
    {
        const std::string text = ConfS(key, std::to_string(fallback));
        if (text == "0")
            return 0u;
        return static_cast<uint32>(std::strtoul(text.c_str(), nullptr, 10));
    };
    bands.Fortified = band("MythicPlus.Modifier.Fortified", 2);
    bands.Tyrannical = band("MythicPlus.Modifier.Tyrannical", 4);
    bands.Raging = band("MythicPlus.Modifier.Raging", 7);
    bands.Bolstering = band("MythicPlus.Modifier.Bolstering", 10);
    bands.Savage = band("MythicPlus.Modifier.Savage", 12);
    return bands;
}

std::string Join(const std::vector<std::string>& names)
{
    std::string text;
    for (const std::string& name : names)
    {
        if (!text.empty())
            text += ",";
        text += name;
    }
    return text.empty() ? "None" : text;
}
}

MythicPlusMgr* MythicPlusMgr::Instance()
{
    static MythicPlusMgr instance;
    return &instance;
}

void MythicPlusMgr::Initialize()
{
    MPConfig::Instance()->LoadFromServer();
    LoadPools();
    LoadStats();
    if (QueryResult result = CharacterDatabase.Query("SELECT season_id FROM mp_season WHERE id = 1"))
        _seasonId = result->Fetch()[0].Get<uint32>();
}

uint32 MythicPlusMgr::CurrentWeekId() const
{
    const time_t now = time(nullptr);
    const int resetDay = static_cast<int>(ConfU("MythicPlus.ResetWeekday", 3));
    const int resetHour = static_cast<int>(ConfU("MythicPlus.ResetHour", 8));
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    const int daysBack = (local.tm_wday - resetDay + 7) % 7;
    time_t guess = now - static_cast<time_t>(daysBack) * 86400;
    std::tm boundary{};
#ifdef _WIN32
    localtime_s(&boundary, &guess);
#else
    localtime_r(&guess, &boundary);
#endif
    boundary.tm_hour = resetHour;
    boundary.tm_min = 0;
    boundary.tm_sec = 0;
    time_t stamp = mktime(&boundary);
    if (stamp > now)
        stamp -= 7 * 86400;
    return static_cast<uint32>(stamp / 3600);
}

void MythicPlusMgr::LoadPools()
{
    _trash.clear();
    _bosses.clear();
    QueryResult result = WorldDatabase.Query(
        "SELECT entry, type, `rank` FROM creature_template "
        "WHERE type > 0 AND type <= 10 AND type != 8 AND npcflag = 0 "
        "AND ScriptName = '' AND VehicleId = 0 AND `rank` != 3");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        PoolEntry entry;
        entry.Entry = fields[0].Get<uint32>();
        entry.Type = fields[1].Get<uint32>();
        const uint8 rank = fields[2].Get<uint8>();
        if (rank == 1 || rank == 2 || rank == 4)
            _bosses[entry.Type].push_back(entry);
        else
            _trash[entry.Type].push_back(entry);
    } while (result->NextRow());

    LOG_INFO("module", "MythicPlus: creature pools loaded");
}

void MythicPlusMgr::LoadStats()
{
    WorldDatabase.Query("SELECT level, class, basehp0 FROM creature_classlevelstats WHERE level = 80 LIMIT 1");
}

KeyRecord MythicPlusMgr::LoadKey(uint32 guidLow)
{
    KeyRecord key;
    std::string sql = "SELECT key_level, week_id, finishes, best_timed, any_timed, vault_level FROM mp_key WHERE guid = " + std::to_string(guidLow);
    if (QueryResult result = CharacterDatabase.Query(sql.c_str()))
    {
        Field* fields = result->Fetch();
        key.Level = fields[0].Get<uint32>();
        key.WeekId = fields[1].Get<uint32>();
        key.Finishes = fields[2].Get<uint32>();
        key.BestTimed = fields[3].Get<uint32>();
        key.AnyTimed = fields[4].Get<uint8>() != 0;
        key.VaultLevel = fields[5].Get<uint32>();
        key.Exists = true;
    }
    return key;
}

void MythicPlusMgr::EnsureCurrentWeek(uint32 guidLow)
{
    KeyRecord key = LoadKey(guidLow);
    ApplyResetIfDue(guidLow, key, GetRunByPlayer(ObjectGuid::Create<HighGuid::Player>(guidLow)) != nullptr);
}

bool MythicPlusMgr::SetHeldKey(uint32 guidLow, uint32 level)
{
    const uint32 floor = ConfU("MythicPlus.KeyFloor", 2);
    if (level < floor)
        return false;
    KeyRecord key = LoadKey(guidLow);
    key.Level = level;
    key.WeekId = CurrentWeekId();
    key.Exists = true;
    SaveKey(guidLow, key);
    return true;
}

void MythicPlusMgr::SaveKey(uint32 guidLow, const KeyRecord& key)
{
    std::ostringstream sql;
    sql << "REPLACE INTO mp_key (guid, key_level, week_id, finishes, best_timed, any_timed, vault_level) VALUES ("
        << guidLow << "," << key.Level << "," << key.WeekId << "," << key.Finishes << ","
        << key.BestTimed << "," << (key.AnyTimed ? 1 : 0) << "," << key.VaultLevel << ")";
    CharacterDatabase.Execute(sql.str().c_str());
}

void MythicPlusMgr::ApplyResetIfDue(uint32 guidLow, KeyRecord& key, bool runStillOpen)
{
    const uint32 week = CurrentWeekId();
    if (!key.Exists)
    {
        key.Level = 2;
        key.WeekId = week;
        key.Exists = true;
        SaveKey(guidLow, key);
        return;
    }
    if (key.WeekId == week || runStillOpen)
        return;

    const uint32 floor = ConfU("MythicPlus.KeyFloor", 2);
    uint32 vaultKey = floor;
    if (key.Finishes > 0)
        vaultKey = std::max(floor, key.VaultLevel ? key.VaultLevel : VaultKeyLevel(key.AnyTimed, key.BestTimed, key.Level, key.Finishes, floor));
    std::ostringstream vault;
    vault << "REPLACE INTO mp_vault (guid, week_id, vault_key, choices, claimed) VALUES ("
          << guidLow << "," << key.WeekId << "," << vaultKey << ","
          << VaultChoiceCount(key.Finishes) << ",0)";
    CharacterDatabase.Execute(vault.str().c_str());

    key.Level = vaultKey;
    key.WeekId = week;
    key.Finishes = 0;
    key.BestTimed = 0;
    key.VaultLevel = 0;
    key.AnyTimed = false;
    SaveKey(guidLow, key);
}

uint32 MythicPlusMgr::PickEntry(int32 creatureType, bool boss) const
{
    std::vector<uint32> entries;
    auto collect = [&](const std::unordered_map<uint32, std::vector<PoolEntry>>& pool)
    {
        if (creatureType < 0)
        {
            for (const auto& pair : pool)
                for (const PoolEntry& entry : pair.second)
                    entries.push_back(entry.Entry);
            return;
        }
        auto it = pool.find(static_cast<uint32>(creatureType));
        if (it == pool.end())
            return;
        for (const PoolEntry& entry : it->second)
            entries.push_back(entry.Entry);
    };

    collect(boss ? _bosses : _trash);
    if (entries.empty() && boss)
        collect(_trash);
    if (entries.empty())
        return 0;
    return entries[urand(0, static_cast<uint32>(entries.size() - 1))];
}

uint32 MythicPlusMgr::ActiveModifierCount(uint32 keyLevel) const
{
    return static_cast<uint32>(ModifiersForKey(keyLevel, LoadBands()).size());
}

bool MythicPlusMgr::TryStart(Player* confirmer, uint32 themeId, uint32 mapId, std::string& error)
{
    if (!confirmer || !confirmer->GetGroup())
    {
        error = "A keyed run needs a group of five level 80 players.";
        return false;
    }

    Group* group = confirmer->GetGroup();
    uint32 count = 0;
    uint32 below = 0;
    std::vector<Player*> members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;
        ++count;
        if (member->GetLevel() < 80)
            ++below;
        members.push_back(member);
    }

    if (!GroupMayStart(count, below))
    {
        error = "A keyed run needs a group of five level 80 players.";
        return false;
    }

    const Catalog& catalog = MPConfig::Instance()->GetCatalog();
    const ThemeDef* theme = nullptr;
    const DungeonDef* dungeon = nullptr;
    for (const ThemeDef& row : catalog.Themes)
        if (row.Id == themeId)
            theme = &row;
    for (const DungeonDef& row : catalog.Dungeons)
        if (row.MapId == mapId)
            dungeon = &row;
    if (!theme || !dungeon)
    {
        error = "That theme or dungeon is not in this season.";
        return false;
    }

    const uint32 guidLow = confirmer->GetGUID().GetCounter();
    KeyRecord key = LoadKey(guidLow);
    const uint32 floor = ConfU("MythicPlus.KeyFloor", 2);
    if (key.Level < floor)
        key.Level = floor;

    Run run;
    run.Id = _nextRunId++;
    run.KeyHolder = confirmer->GetGUID();
    run.KeyLevel = key.Level;
    run.ThemeId = themeId;
    run.MapId = mapId;
    run.WeekId = key.WeekId;
    run.StartedAt = static_cast<uint64>(GameTime::GetGameTime().count());
    run.ModifierText = Join(ModifiersForKey(key.Level, LoadBands()));

    QueryResult entrance = WorldDatabase.Query(
        "SELECT target_position_x, target_position_y, target_position_z, target_orientation "
        "FROM areatrigger_teleport WHERE target_map = {} LIMIT 1", mapId);
    if (!entrance)
    {
        error = "This dungeon has no entrance on the server.";
        return false;
    }
    Field* fields = entrance->Fetch();
    run.EntranceX = fields[0].Get<float>();
    run.EntranceY = fields[1].Get<float>();
    run.EntranceZ = fields[2].Get<float>();
    run.EntranceO = fields[3].Get<float>();

    MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
    const Difficulty difficulty = (mapEntry && mapEntry->Expansion() >= 1)
        ? DUNGEON_DIFFICULTY_HEROIC
        : DUNGEON_DIFFICULTY_NORMAL;

    for (Player* member : members)
    {
        sInstanceSaveMgr->PlayerUnbindInstance(member->GetGUID(), mapId, DUNGEON_DIFFICULTY_NORMAL, true, member);
        sInstanceSaveMgr->PlayerUnbindInstance(member->GetGUID(), mapId, DUNGEON_DIFFICULTY_HEROIC, true, member);
        member->SetDungeonDifficulty(difficulty);

        RunPlayer data;
        data.Guid = member->GetGUID();
        data.ReturnMap = member->GetMapId();
        data.ReturnX = member->GetPositionX();
        data.ReturnY = member->GetPositionY();
        data.ReturnZ = member->GetPositionZ();
        data.ReturnO = member->GetOrientation();
        if (!member->IsAlive())
        {
            member->ResurrectPlayer(1.0f);
            member->SpawnCorpseBones();
        }
        if (!member->TeleportTo(mapId, run.EntranceX, run.EntranceY, run.EntranceZ, run.EntranceO, TELE_TO_GM_MODE))
        {
            for (const RunPlayer& sent : run.Players)
                if (Player* earlier = ObjectAccessor::FindPlayer(sent.Guid))
                    earlier->TeleportTo(sent.ReturnMap, sent.ReturnX, sent.ReturnY, sent.ReturnZ, sent.ReturnO, TELE_TO_GM_MODE);
            error = "Teleport failed.";
            return false;
        }
        run.Players.push_back(data);

        ChatHandler(member->GetSession()).SendSysMessage(
            ("|cFF00FF00[Mythic+]|r Key +" + std::to_string(run.KeyLevel) + "  " + dungeon->Name + "  " + run.ModifierText).c_str());
    }

    _runs[run.Id] = run;
    for (const RunPlayer& member : run.Players)
        _playerRuns[member.Guid.GetRawValue()] = run.Id;
    SendRunState(_runs[run.Id]);
    return true;
}

void MythicPlusMgr::NoteDeath(Player* player)
{
    if (!player)
        return;
    Run* run = GetRunByPlayer(player->GetGUID());
    if (!run || player->GetMapId() != run->MapId)
        return;
    ++run->Deaths;
    player->SetFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_NO_RELEASE_WINDOW);
    player->RemoveFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_RELEASE_TIMER);
    SendRunState(*run);
}

void MythicPlusMgr::Abort(Player* player)
{
    if (!player)
        return;
    Run* run = GetRunByPlayer(player->GetGUID());
    if (!run)
        return;
    run->Resolved = true;
    const uint32 runId = run->Id;
    for (const RunPlayer& member : run->Players)
    {
        _playerRuns.erase(member.Guid.GetRawValue());
        if (Player* memberPlayer = ObjectAccessor::FindPlayer(member.Guid))
        {
            memberPlayer->RemoveFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_NO_RELEASE_WINDOW);
            if (!memberPlayer->IsAlive())
            {
                memberPlayer->ResurrectPlayer(1.0f);
                memberPlayer->SpawnCorpseBones();
            }
            memberPlayer->TeleportTo(member.ReturnMap, member.ReturnX, member.ReturnY, member.ReturnZ, member.ReturnO, TELE_TO_GM_MODE);
        }
    }
    _runs.erase(runId);
}

void MythicPlusMgr::TryPopulate(Player* player)
{
    if (!player)
        return;
    Run* run = GetRunByPlayer(player->GetGUID());
    if (!run || run->Populated || player->GetMapId() != run->MapId)
        return;
    Map* map = player->GetMap();
    if (InstanceMap* instance = map ? map->ToInstanceMap() : nullptr)
        Populate(*run, instance);
}

Run* MythicPlusMgr::GetRunByPlayer(ObjectGuid guid)
{
    auto it = _playerRuns.find(guid.GetRawValue());
    if (it == _playerRuns.end())
        return nullptr;
    auto run = _runs.find(it->second);
    if (run == _runs.end() || run->second.Resolved)
        return nullptr;
    return &run->second;
}

void MythicPlusMgr::Broadcast(Player* player, const std::string& payload) const
{
    if (!player || !player->GetSession())
        return;
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_ADDON, player, player, payload);
    player->GetSession()->SendPacket(&data);
}

void MythicPlusMgr::SendRunState(const Run& run)
{
    const uint64 now = static_cast<uint64>(GameTime::GetGameTime().count());
    const uint32 elapsed = now > run.StartedAt ? static_cast<uint32>(now - run.StartedAt) + run.Deaths * ConfU("MythicPlus.DeathPenaltySeconds", 5) : 0;
    const uint32 limit = ConfU("MythicPlus.TimerSeconds", 1800);
    const bool expired = elapsed >= limit;
    const uint32 floor = ConfU("MythicPlus.KeyFloor", 2);
    for (const RunPlayer& member : run.Players)
    {
        Player* player = ObjectAccessor::FindPlayer(member.Guid);
        if (!player)
            continue;
        const KeyRecord key = LoadKey(member.Guid.GetCounter());
        const uint32 vault = key.Finishes == 0 ? floor : std::max(floor, key.VaultLevel ? key.VaultLevel : key.Level);
        const std::string payload = "MythicPlus|run|" + std::to_string(run.KeyLevel) + "|" + run.ModifierText + "|"
            + std::to_string(expired ? 0 : limit - elapsed) + "|" + (expired ? "expired" : "running") + "|"
            + std::to_string(key.Level) + "|" + std::to_string(vault);
        Broadcast(player, payload);
    }
}

bool MythicPlusMgr::Populate(Run& run, InstanceMap* map)
{
    if (!map)
        return false;

    std::vector<Creature*> existing;
    for (auto const& pair : map->GetCreatureBySpawnIdStore())
        if (pair.second && pair.second->IsInWorld() && !pair.second->IsPet())
            existing.push_back(pair.second);
    for (Creature* creature : existing)
    {
        creature->SetRespawnTime(7 * DAY);
        creature->DespawnOrUnsummon();
    }

    std::vector<GameObject*> doors;
    for (auto const& pair : map->GetGameObjectBySpawnIdStore())
    {
        GameObject* object = pair.second;
        if (object && object->IsInWorld() && (object->GetGoType() == GAMEOBJECT_TYPE_DOOR || object->GetGoType() == GAMEOBJECT_TYPE_BUTTON))
            doors.push_back(object);
    }
    for (GameObject* object : doors)
        object->Delete();

    if (InstanceScript* script = map->GetInstanceScript())
    {
        for (uint32 i = 0; i < 25; ++i)
        {
            if (script->GetBossState(i) == TO_BE_DECIDED)
                break;
            script->SetBossState(i, DONE);
        }
    }

    QueryResult points = WorldDatabase.Query(
        "SELECT position_x, position_y, position_z, orientation FROM creature WHERE map = {}", run.MapId);
    const Catalog& catalog = MPConfig::Instance()->GetCatalog();
    int32 creatureType = -1;
    for (const ThemeDef& theme : catalog.Themes)
        if (theme.Id == run.ThemeId && !theme.CreatureTypes.empty())
            creatureType = theme.CreatureTypes.front();

    const auto names = ModifiersForKey(run.KeyLevel, LoadBands());
    const float base = sConfigMgr->GetOption<float>("MythicPlus.Scaling.BaseMultiplier", 1.10f);
    const float perKey = sConfigMgr->GetOption<float>("MythicPlus.Scaling.PerKeyMultiplier", 0.10f);
    const float keyMult = base + perKey * (run.KeyLevel > 2 ? run.KeyLevel - 2 : 0);
    const float fortified = IsFortified(names) ? 1.20f : 1.0f;
    const float tyrannical = IsTyrannical(names) ? 1.40f : 1.0f;

    auto scale = [&](Creature* creature, bool boss)
    {
        creature->SetLevel(80);
        creature->SetFaction(14);
        creature->SetReactState(REACT_AGGRESSIVE);
        const float role = boss ? tyrannical * 3.0f : fortified;
        const uint32 health = static_cast<uint32>(std::max(1.0f, creature->GetMaxHealth() * keyMult * role));
        creature->SetMaxHealth(health);
        creature->SetHealth(health);
        creature->setActive(true);
        float damage = keyMult;
        if (boss && tyrannical > 1.0f)
            damage *= 1.15f;
        if (names.end() != std::find(names.begin(), names.end(), "Savage"))
            damage *= 1.20f;
        NoteCreatureLevel(creature->GetGUID(), 80, health, map->GetInstanceId(), damage);
        SpawnedMob mob;
        mob.Guid = creature->GetGUID();
        mob.IsBoss = boss;
        run.Mobs.push_back(mob);
        if (boss)
            ++run.Bosses;
        else
            ++run.Spawned;
    };

    uint32 spawnedTrash = 0;
    if (points)
    {
        do
        {
            Field* fields = points->Fetch();
            Position pos(fields[0].Get<float>(), fields[1].Get<float>(), fields[2].Get<float>(), fields[3].Get<float>());
            const uint32 entry = PickEntry(creatureType, false);
            if (!entry)
                continue;
            if (Creature* creature = map->SummonCreature(entry, pos))
            {
                scale(creature, false);
                ++spawnedTrash;
            }
        } while (points->NextRow());
    }

    QueryResult bosses = WorldDatabase.Query(BuildBossSpawnPointQuery(run.MapId).c_str());
    if (bosses)
    {
        uint32 spawnedBosses = 0;
        do
        {
            if (spawnedBosses >= 4)
                break;
            Field* fields = bosses->Fetch();
            Position pos(fields[0].Get<float>(), fields[1].Get<float>(), fields[2].Get<float>(), fields[3].Get<float>());
            if (const uint32 entry = PickEntry(creatureType, true))
            {
                if (Creature* creature = map->SummonCreature(entry, pos))
                {
                    scale(creature, true);
                    ++spawnedBosses;
                }
            }
        } while (bosses->NextRow());
    }

    if (InstanceScript* script = map->GetInstanceScript())
    {
        for (uint32 i = 0; i < 25; ++i)
        {
            if (script->GetBossState(i) == TO_BE_DECIDED)
                break;
            if (script->GetBossState(i) == DONE)
                script->SetBossState(i, NOT_STARTED);
        }
    }

    run.Populated = true;
    run.InstanceId = map->GetInstanceId();
    return run.Spawned > 0 || run.Bosses > 0;
}

void MythicPlusMgr::Resolve(Run& run)
{
    if (run.Resolved)
        return;
    run.Resolved = true;
    if (run.Abandoned)
    {
        for (const RunPlayer& member : run.Players)
            _playerRuns.erase(member.Guid.GetRawValue());
        return;
    }

    const uint64 now = static_cast<uint64>(GameTime::GetGameTime().count());
    KeyResolveInput input;
    input.KeyLevel = run.KeyLevel;
    input.LimitSeconds = ConfU("MythicPlus.TimerSeconds", 1800);
    input.ElapsedSeconds = now > run.StartedAt ? static_cast<uint32>(now - run.StartedAt) : 0;
    input.Deaths = run.Deaths;
    input.DeathPenaltySeconds = ConfU("MythicPlus.DeathPenaltySeconds", 5);
    input.BossesDead = run.Bosses > 0 && run.BossesKilled >= run.Bosses;
    input.Spawned = run.Spawned;
    input.Killed = run.Killed;
    input.SharePercent = ConfU("MythicPlus.EnemyForcePercent", 80);
    input.Floor = ConfU("MythicPlus.KeyFloor", 2);
    input.UpgradePercent3 = ConfU("MythicPlus.UpgradePercent.3", 40);
    input.UpgradePercent2 = ConfU("MythicPlus.UpgradePercent.2", 20);
    const KeyResolveResult result = ResolveKey(input);

    const uint32 holder = run.KeyHolder.GetCounter();
    const uint32 groupVault = result.Timed ? result.LevelReached : (result.Finished ? result.NewLevel : 0);
    const uint32 clearSeconds = input.ElapsedSeconds + input.Deaths * input.DeathPenaltySeconds;
    for (const RunPlayer& member : run.Players)
    {
        Player* player = ObjectAccessor::FindPlayer(member.Guid);
        const uint32 low = member.Guid.GetCounter();
        KeyRecord own = LoadKey(low);
        if (low == holder)
            own.Level = result.NewLevel;
        own.WeekId = run.WeekId;
        if (result.Finished)
            ++own.Finishes;
        if (result.Timed && result.LevelReached > own.BestTimed)
        {
            own.AnyTimed = true;
            own.BestTimed = result.LevelReached;
        }
        if (groupVault > own.VaultLevel)
            own.VaultLevel = groupVault;
        SaveKey(low, own);

        if (result.Finished && player)
            GiveGear(player, ItemLevelForKey(run.KeyLevel, ConfU("MythicPlus.Gear.BaseItemLevel", 245), ConfU("MythicPlus.Gear.ItemLevelPerKey", 3)), "Mythic+ chest");

        if (result.Timed)
            RememberBest(low, run.MapId, result.LevelReached, clearSeconds, run.ThemeId, run.KeyLevel, player ? player->GetName() : "");
        else if (result.Finished)
            RememberBest(low, run.MapId, 0, clearSeconds, run.ThemeId, run.KeyLevel, player ? player->GetName() : "");

        if (player && player->GetSession())
        {
            std::string text = "|cFF00FF00[Mythic+]|r ";
            text += result.Timed ? "Timed clear. " : (result.Finished ? "Late clear. " : "Run missed. ");
            text += "Key is now +" + std::to_string(low == holder ? result.NewLevel : LoadKey(low).Level) + ".";
            ChatHandler(player->GetSession()).SendSysMessage(text.c_str());
        }
        _playerRuns.erase(member.Guid.GetRawValue());
    }

    if (run.ResetPending)
    {
        for (const RunPlayer& member : run.Players)
        {
            KeyRecord refreshed = LoadKey(member.Guid.GetCounter());
            refreshed.WeekId = run.WeekId;
            ApplyResetIfDue(member.Guid.GetCounter(), refreshed, false);
        }
    }
}

void MythicPlusMgr::GiveGear(Player* player, uint32 itemLevel, const char* subject)
{
    QueryResult result = WorldDatabase.Query(
        "SELECT entry FROM item_template WHERE ItemLevel = {} AND `class` IN (2, 4) AND Quality >= 3 LIMIT 1",
        itemLevel);
    if (!result)
        result = WorldDatabase.Query(
            "SELECT entry FROM item_template WHERE ItemLevel >= {} AND `class` IN (2, 4) AND Quality >= 2 ORDER BY ItemLevel ASC LIMIT 1",
            itemLevel);
    if (!result)
        return;

    const uint32 entry = result->Fetch()[0].Get<uint32>();
    if (player->AddItem(entry, 1))
        return;

    Item* item = Item::CreateItem(entry, 1, player);
    if (!item)
        return;
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    MailDraft(subject, "Your Mythic+ reward.")
        .AddItem(item)
        .SendMailTo(trans, MailReceiver(player, player->GetGUID().GetCounter()), MailSender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM));
    CharacterDatabase.CommitTransaction(trans);
}

void MythicPlusMgr::RememberBest(uint32 guidLow, uint32 mapId, uint32 levelReached, uint32 clearSeconds,
                                 uint32 themeId, uint32 keyLevel, const std::string& name)
{
    if (levelReached > 0)
    {
        uint32 stored = 0;
        std::string lookup = "SELECT best_key FROM mp_dungeon_best WHERE guid = " + std::to_string(guidLow)
            + " AND map_id = " + std::to_string(mapId) + " AND season_id = " + std::to_string(_seasonId);
        if (QueryResult result = CharacterDatabase.Query(lookup.c_str()))
            stored = result->Fetch()[0].Get<uint32>();
        const uint32 best = RaiseBest(stored, levelReached);
        std::ostringstream sql;
        sql << "REPLACE INTO mp_dungeon_best (guid, season_id, map_id, best_key) VALUES ("
            << guidLow << "," << _seasonId << "," << mapId << "," << best << ")";
        CharacterDatabase.Execute(sql.str().c_str());
    }

    if (clearSeconds == 0)
        return;
    std::string fastest = "SELECT MIN(clear_time) FROM mp_leaderboard WHERE map_id = " + std::to_string(mapId);
    uint32 previous = 0;
    if (QueryResult result = CharacterDatabase.Query(fastest.c_str()))
        if (!result->Fetch()[0].IsNull())
            previous = result->Fetch()[0].Get<uint32>();
    if (previous != 0 && clearSeconds >= previous)
        return;

    const std::string safeName = CharacterDatabase.EscapeString(name);
    std::ostringstream board;
    board << "INSERT INTO mp_leaderboard (guid, char_name, map_id, theme_id, key_level, clear_time) VALUES ("
          << guidLow << ",'" << safeName << "'," << mapId << "," << themeId << "," << keyLevel << "," << clearSeconds << ")";
    CharacterDatabase.Execute(board.str().c_str());
}

void MythicPlusMgr::Update(uint32 diff)
{
    _updateTimer += diff;
    if (_updateTimer < 1000)
        return;
    _updateTimer = 0;

    std::vector<uint32> finished;
    for (auto& pair : _runs)
    {
        Run& run = pair.second;
        if (run.Resolved)
            continue;

        Player* reference = nullptr;
        uint32 present = 0;
        for (const RunPlayer& member : run.Players)
        {
            Player* player = ObjectAccessor::FindPlayer(member.Guid);
            if (player && player->IsInWorld() && player->GetMapId() == run.MapId)
            {
                ++present;
                reference = player;
            }
        }
        if (!reference)
        {
            const uint64 now = static_cast<uint64>(GameTime::GetGameTime().count());
            if (!run.Arrived && now > run.StartedAt && now - run.StartedAt > 120)
                run.Abandoned = true;
            if (run.Arrived || run.Abandoned)
                finished.push_back(run.Id);
            continue;
        }
        run.Arrived = true;

        Map* map = reference->GetMap();
        InstanceMap* instance = map ? map->ToInstanceMap() : nullptr;
        if (!run.Populated && instance)
            Populate(run, instance);

        for (SpawnedMob& mob : run.Mobs)
        {
            if (mob.Credited || !instance)
                continue;
            Creature* creature = instance->GetCreature(mob.Guid);
            if (!creature || creature->IsAlive())
                continue;
            mob.Credited = true;
            StripEquipableLoot(creature);
            if (mob.IsBoss)
                ++run.BossesKilled;
            else
                ++run.Killed;
            if (run.ModifierText.find("Bolstering") != std::string::npos)
            {
                for (const SpawnedMob& other : run.Mobs)
                {
                    if (other.Guid == mob.Guid || other.Credited)
                        continue;
                    Creature* ally = instance->GetCreature(other.Guid);
                    if (!ally || !ally->IsAlive())
                        continue;
                    const uint32 oldMax = ally->GetMaxHealth();
                    const uint32 health = static_cast<uint32>(oldMax * 1.15f);
                    const uint32 current = ally->GetHealth();
                    ally->SetMaxHealth(health);
                    ally->SetHealth(std::min(health, current + (health - oldMax)));
                    uint8 level = 80;
                    uint32 stored = health;
                    if (OwnedLevel(ally->GetGUID(), level, stored))
                        NoteCreatureLevel(ally->GetGUID(), level, health, instance->GetInstanceId(), OwnedDamage(ally->GetGUID()));
                }
            }
        }

        bool inCombat = reference->IsInCombat();
        if (!inCombat)
        {
            for (const RunPlayer& member : run.Players)
                if (Player* player = ObjectAccessor::FindPlayer(member.Guid))
                    if (player->IsInCombat())
                        inCombat = true;
        }
        if (!inCombat)
        {
            for (const RunPlayer& member : run.Players)
            {
                Player* player = ObjectAccessor::FindPlayer(member.Guid);
                if (!player || player->IsAlive() || player->GetMapId() != run.MapId)
                    continue;
                player->RemoveFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_NO_RELEASE_WINDOW);
                player->ResurrectPlayer(1.0f);
                player->SpawnCorpseBones();
                player->TeleportTo(run.MapId, run.EntranceX, run.EntranceY, run.EntranceZ, run.EntranceO, TELE_TO_GM_MODE);
                if (player->GetSession())
                    ChatHandler(player->GetSession()).SendSysMessage("|cFF00FF00[Mythic+]|r Revived at the entrance.");
            }
        }

        const uint32 week = CurrentWeekId();
        if (week != run.WeekId)
            run.ResetPending = true;

        const bool bossesDone = run.Bosses > 0 && run.BossesKilled >= run.Bosses;
        const uint32 needed = run.Spawned == 0 ? 1 : (run.Spawned * ConfU("MythicPlus.EnemyForcePercent", 80) + 99) / 100;
        const bool forceMiss = run.Populated && (run.Spawned == 0 || run.Bosses == 0);
        if (run.Populated && (forceMiss || (bossesDone && run.Killed >= needed) || present == 0))
            finished.push_back(run.Id);
        else
            SendRunState(run);
    }

    for (uint32 id : finished)
    {
        auto it = _runs.find(id);
        if (it == _runs.end())
            continue;
        Resolve(it->second);
        _runs.erase(it);
    }
}

void MythicPlusMgr::StripEquipableLoot(Creature* creature) const
{
    if (!creature)
        return;
    std::vector<LootItem> kept;
    for (const LootItem& item : creature->loot.items)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemid);
        if (proto && (proto->Class == ITEM_CLASS_WEAPON || proto->Class == ITEM_CLASS_ARMOR))
            continue;
        kept.push_back(item);
    }
    creature->loot.items.swap(kept);
}

void MythicPlusMgr::NoteCreatureLevel(ObjectGuid guid, uint8 level, uint32 health, uint32 instanceId, float damageMult)
{
    _owned[guid] = Owned{ level, health, instanceId, damageMult };
}

float MythicPlusMgr::OwnedDamage(ObjectGuid guid) const
{
    auto it = _owned.find(guid);
    return it == _owned.end() ? 1.0f : it->second.Damage;
}

bool MythicPlusMgr::OwnedLevel(ObjectGuid guid, uint8& level, uint32& health) const
{
    auto it = _owned.find(guid);
    if (it == _owned.end())
        return false;
    level = it->second.Level;
    health = it->second.Health;
    return true;
}

void MythicPlusMgr::ForgetCreature(ObjectGuid guid)
{
    _owned.erase(guid);
}

std::string MythicPlusMgr::ScoreText(uint32 guidLow) const
{
    std::vector<uint32> bests;
    std::string sql = "SELECT best_key FROM mp_dungeon_best WHERE guid = " + std::to_string(guidLow) + " AND season_id = " + std::to_string(_seasonId);
    if (QueryResult result = CharacterDatabase.Query(sql.c_str()))
    {
        do
            bests.push_back(result->Fetch()[0].Get<uint32>());
        while (result->NextRow());
    }
    return "Score " + std::to_string(MythicScore(bests, ConfU("MythicPlus.Score.PerKey", 10)));
}

std::string MythicPlusMgr::BoardText(uint32 mapId) const
{
    std::string sql = "SELECT char_name, clear_time, key_level FROM mp_leaderboard WHERE map_id = "
        + std::to_string(mapId) + " ORDER BY clear_time ASC LIMIT 5";
    QueryResult result = CharacterDatabase.Query(sql.c_str());
    if (!result)
        return "No clears yet.";
    std::string text;
    do
    {
        Field* fields = result->Fetch();
        if (!fields[0].IsNull())
            text += fields[0].Get<std::string>() + " " + std::to_string(fields[1].Get<uint32>()) + "s +"
                + std::to_string(fields[2].Get<uint32>()) + "\n";
    } while (result->NextRow());
    return text;
}

std::string MythicPlusMgr::VaultText(uint32 guidLow) const
{
    std::string sql = "SELECT vault_key, choices, claimed FROM mp_vault WHERE guid = " + std::to_string(guidLow)
        + " ORDER BY week_id DESC LIMIT 1";
    QueryResult result = CharacterDatabase.Query(sql.c_str());
    if (!result)
        return "Vault is empty.";
    Field* fields = result->Fetch();
    return "Vault key +" + std::to_string(fields[0].Get<uint32>()) + ", choices " + std::to_string(fields[1].Get<uint32>());
}

bool MythicPlusMgr::ClaimVaultChoice(Player* player, uint32 index)
{
    if (!player || index >= 3)
        return false;
    const uint32 low = player->GetGUID().GetCounter();
    std::string sql = "SELECT week_id, vault_key, choices, claimed FROM mp_vault WHERE guid = " + std::to_string(low) + " ORDER BY week_id DESC LIMIT 1";
    QueryResult result = CharacterDatabase.Query(sql.c_str());
    if (!result)
        return false;
    Field* fields = result->Fetch();
    const uint32 choices = fields[2].Get<uint32>();
    const uint32 claimed = fields[3].Get<uint32>();
    if (index >= choices || (claimed & (1u << index)))
        return false;
    GiveGear(player, ItemLevelForKey(fields[1].Get<uint32>(), ConfU("MythicPlus.Gear.BaseItemLevel", 245), ConfU("MythicPlus.Gear.ItemLevelPerKey", 3)), "Mythic+ vault");
    std::ostringstream update;
    update << "UPDATE mp_vault SET claimed = " << (claimed | (1u << index)) << " WHERE guid = " << low << " AND week_id = " << fields[0].Get<uint32>();
    CharacterDatabase.Execute(update.str().c_str());
    return true;
}

void MythicPlusMgr::StartSeason()
{
    ++_seasonId;
    CharacterDatabase.Execute("REPLACE INTO mp_season (id, season_id) VALUES (1, {})", _seasonId);
    MPConfig::Instance()->LoadFromServer();
}

} // namespace MythicPlus
