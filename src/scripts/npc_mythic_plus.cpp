#include "MythicPlusMgr.h"
#include "MPConfig.h"

#include "Chat.h"
#include "Config.h"
#include "Group.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

#include <algorithm>
#include <string>
#include <unordered_map>

using namespace MythicPlus;

namespace
{
struct Pending
{
    uint32 ThemeId = 0;
    uint32 MapId = 0;
};

std::unordered_map<uint64, Pending> PendingChoice;

bool Enabled()
{
    return sConfigMgr->GetOption<uint32>("MythicPlus.Enable", 1) != 0;
}

void ShowHello(Player* player, Creature* creature)
{
    const uint32 low = player->GetGUID().GetCounter();
    sMythicPlus->EnsureCurrentWeek(low);
    const KeyRecord key = sMythicPlus->LoadKey(low);
    const uint32 floor = sConfigMgr->GetOption<uint32>("MythicPlus.KeyFloor", 2);
    const uint32 vault = key.Finishes == 0 ? floor : std::max(floor, key.VaultLevel ? key.VaultLevel : key.Level);
    std::string text = "Held key +" + std::to_string(key.Level) + ". Vault key +" + std::to_string(vault)
        + ". " + sMythicPlus->ScoreText(low) + ". " + sMythicPlus->VaultText(low);
    ChatHandler(player->GetSession()).SendSysMessage(text.c_str());

    const Catalog& catalog = MPConfig::Instance()->GetCatalog();
    uint32 index = 0;
    for (const ThemeDef& theme : catalog.Themes)
    {
        AddGossipItemFor(player, 0, theme.Name, GOSSIP_SENDER_MAIN, 1000 + index);
        ++index;
    }
    for (uint32 choice = 0; choice < 3; ++choice)
        AddGossipItemFor(player, 0, "Take vault choice " + std::to_string(choice + 1), GOSSIP_SENDER_MAIN, 3000 + choice);
    SendGossipMenuFor(player, 1, creature);
}
}

class npc_mythic_plus : public CreatureScript
{
public:
    npc_mythic_plus() : CreatureScript("npc_mythic_plus") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!Enabled())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Mythic+ is turned off.");
            return true;
        }
        ClearGossipMenuFor(player);
        ShowHello(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!Enabled())
            return true;

        ClearGossipMenuFor(player);
        const Catalog& catalog = MPConfig::Instance()->GetCatalog();
        Pending& pending = PendingChoice[player->GetGUID().GetRawValue()];

        if (action >= 1000 && action < 2000)
        {
            const uint32 index = action - 1000;
            if (index >= catalog.Themes.size())
                return true;
            pending.ThemeId = catalog.Themes[index].Id;
            action = 2000;
        }

        if (action >= 2000 && action < 2200)
        {
            const uint32 page = action - 2000;
            const uint32 start = page * 20;
            if (pending.ThemeId == 0 || start >= catalog.Dungeons.size())
                return true;
            const uint32 end = std::min<uint32>(start + 20, static_cast<uint32>(catalog.Dungeons.size()));
            for (uint32 dungeonIndex = start; dungeonIndex < end; ++dungeonIndex)
                AddGossipItemFor(player, 0, catalog.Dungeons[dungeonIndex].Name, GOSSIP_SENDER_MAIN, 2200 + dungeonIndex);
            if (end < catalog.Dungeons.size())
                AddGossipItemFor(player, 0, "More dungeons", GOSSIP_SENDER_MAIN, 2000 + page + 1);
            SendGossipMenuFor(player, 1, creature);
            return true;
        }

        if (action >= 2200 && action < 3000)
        {
            const uint32 index = action - 2200;
            if (index >= catalog.Dungeons.size() || pending.ThemeId == 0)
                return true;
            pending.MapId = catalog.Dungeons[index].MapId;
            ChatHandler(player->GetSession()).SendSysMessage(sMythicPlus->BoardText(pending.MapId).c_str());
            AddGossipItemFor(player, 0, "Start this key", GOSSIP_SENDER_MAIN, 6000);
            AddGossipItemFor(player, 0, "Back", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(player, 1, creature);
            return true;
        }

        if (action >= 3000 && action < 4000)
        {
            if (!sMythicPlus->ClaimVaultChoice(player, action - 3000))
                ChatHandler(player->GetSession()).SendSysMessage("That vault choice is not open.");
            ShowHello(player, creature);
            return true;
        }

        if (action == 6000)
        {
            std::string error;
            if (!sMythicPlus->TryStart(player, pending.ThemeId, pending.MapId, error))
                ChatHandler(player->GetSession()).SendSysMessage(error.c_str());
            CloseGossipMenuFor(player);
            return true;
        }

        ShowHello(player, creature);
        return true;
    }
};

void AddSC_npc_mythic_plus()
{
    new npc_mythic_plus();
}
