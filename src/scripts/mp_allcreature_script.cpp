#include "MythicPlusMgr.h"

#include "Creature.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"

#include <algorithm>

using namespace MythicPlus;

class mp_allcreature_script : public AllCreatureScript
{
public:
    mp_allcreature_script() : AllCreatureScript("mp_allcreature_script") {}

    void OnBeforeCreatureSelectLevel(CreatureTemplate const* /*cinfo*/, Creature* creature, uint8& level) override
    {
        uint8 owned = 0;
        uint32 health = 0;
        if (creature && sMythicPlus->OwnedLevel(creature->GetGUID(), owned, health) && owned)
            level = owned;
    }

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature || !sMythicPlus->HasOwned())
            return;
        uint8 level = 0;
        uint32 health = 0;
        if (!sMythicPlus->OwnedLevel(creature->GetGUID(), level, health))
            return;
        if (creature->GetLevel() != level)
            creature->SetLevel(level);
        if (health && creature->GetMaxHealth() != health)
        {
            const uint32 current = creature->GetHealth();
            creature->SetMaxHealth(health);
            creature->SetHealth(std::min(health, current));
        }
        if (!creature->IsAlive())
            sMythicPlus->StripEquipableLoot(creature);
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (creature)
            sMythicPlus->ForgetCreature(creature->GetGUID());
    }
};

void AddSC_mp_allcreature_script()
{
    new mp_allcreature_script();
}
