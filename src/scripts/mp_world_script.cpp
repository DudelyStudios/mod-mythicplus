#include "MythicPlusMgr.h"
#include "MPConfig.h"

#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"

using namespace MythicPlus;

class mp_world_script : public WorldScript
{
public:
    mp_world_script() : WorldScript("mp_world_script") {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        MPConfig::Instance()->LoadFromServer();
    }

    void OnStartup() override
    {
        if (sConfigMgr->GetOption<uint32>("MythicPlus.Enable", 1) == 0)
        {
            LOG_INFO("module", "MythicPlus: disabled in configuration.");
            return;
        }
        sMythicPlus->Initialize();
    }

    void OnUpdate(uint32 diff) override
    {
        if (sConfigMgr->GetOption<uint32>("MythicPlus.Enable", 1) == 0)
            return;
        sMythicPlus->Update(diff);
    }
};

void AddSC_mp_world_script()
{
    new mp_world_script();
}
