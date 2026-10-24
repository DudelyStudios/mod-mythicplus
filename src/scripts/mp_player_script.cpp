#include "MythicPlusMgr.h"

#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace MythicPlus;

class mp_player_script : public PlayerScript
{
public:
    mp_player_script() : PlayerScript("mp_player_script") {}

    void OnPlayerJustDied(Player* player) override
    {
        if (sConfigMgr->GetOption<uint32>("MythicPlus.Enable", 1) == 0)
            return;
        sMythicPlus->NoteDeath(player);
    }

    bool OnPlayerCanRepopAtGraveyard(Player* player) override
    {
        if (player && sMythicPlus->GetRunByPlayer(player->GetGUID()))
            return false;
        return true;
    }
};

void AddSC_mp_player_script()
{
    new mp_player_script();
}
