#include "MythicPlusMgr.h"

#include "Config.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"

class mp_allmap_script : public AllMapScript
{
public:
    mp_allmap_script() : AllMapScript("mp_allmap_script") {}

    void OnPlayerEnterAll(Map* map, Player* player) override
    {
        if (!map || !player || !map->IsDungeon())
            return;
        if (sConfigMgr->GetOption<uint32>("MythicPlus.Enable", 1) == 0)
            return;
        sMythicPlus->TryPopulate(player);
    }
};

void AddSC_mp_allmap_script()
{
    new mp_allmap_script();
}
