#include "ScriptMgr.h"
#include "Log.h"

void AddSC_npc_mythic_plus();
void AddSC_mp_player_script();
void AddSC_mp_world_script();
void AddSC_mp_allmap_script();
void AddSC_mp_command_script();
void AddSC_mp_unit_script();
void AddSC_mp_allcreature_script();

void Addmod_mythicplusScripts()
{
    LOG_INFO("module", "MythicPlus: registering scripts");
    AddSC_npc_mythic_plus();
    AddSC_mp_player_script();
    AddSC_mp_world_script();
    AddSC_mp_allmap_script();
    AddSC_mp_command_script();
    AddSC_mp_unit_script();
    AddSC_mp_allcreature_script();
}
