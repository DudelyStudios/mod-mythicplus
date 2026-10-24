#include "MythicPlusMgr.h"
#include "MPConfig.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;
using namespace MythicPlus;

class mp_command_script : public CommandScript
{
public:
    mp_command_script() : CommandScript("mp_command_script") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable table =
        {
            { "reload", HandleReload, SEC_ADMINISTRATOR, Console::Yes },
            { "status", HandleStatus, SEC_GAMEMASTER, Console::Yes },
            { "key", HandleKey, SEC_GAMEMASTER, Console::No },
            { "season", HandleSeason, SEC_ADMINISTRATOR, Console::Yes },
            { "end", HandleEnd, SEC_GAMEMASTER, Console::No },
        };
        static ChatCommandTable root = { { "mp", table } };
        return root;
    }

    static bool HandleReload(ChatHandler* handler)
    {
        MPConfig::Instance()->LoadFromServer();
        handler->SendSysMessage("Mythic+: catalog reloaded.");
        return true;
    }

    static bool HandleStatus(ChatHandler* handler)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player && handler->GetSession())
            player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Select a player.");
            return true;
        }
        const KeyRecord key = sMythicPlus->LoadKey(player->GetGUID().GetCounter());
        handler->PSendSysMessage("Mythic+: {} holds +{}. Season {}. {}",
            player->GetName(), key.Level, sMythicPlus->SeasonId(), sMythicPlus->ScoreText(player->GetGUID().GetCounter()));
        return true;
    }

    static bool HandleKey(ChatHandler* handler, uint32 level)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player && handler->GetSession())
            player = handler->GetSession()->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Select a player.");
            return true;
        }
        if (!sMythicPlus->SetHeldKey(player->GetGUID().GetCounter(), level))
        {
            handler->SendSysMessage("That key is below the floor. The stored key was left in place.");
            return true;
        }
        handler->PSendSysMessage("Mythic+: {} now holds +{}.", player->GetName(), level);
        return true;
    }

    static bool HandleSeason(ChatHandler* handler)
    {
        sMythicPlus->StartSeason();
        handler->PSendSysMessage("Mythic+: season {} started. Scores cleared. Held keys stay.", sMythicPlus->SeasonId());
        return true;
    }

    static bool HandleEnd(ChatHandler* handler)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
            return true;
        sMythicPlus->Abort(player);
        handler->SendSysMessage("Mythic+: sent the run back to where it started. The key was left in place.");
        return true;
    }
};

void AddSC_mp_command_script()
{
    new mp_command_script();
}
