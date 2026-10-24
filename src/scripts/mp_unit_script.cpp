#include "MythicPlusMgr.h"

#include "Creature.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "Unit.h"

#include <algorithm>
#include <string>

using namespace MythicPlus;

namespace
{
void Scale(Unit* target, Unit* attacker, uint32& damage)
{
    if (!target || !attacker || !damage)
        return;

    Player* player = target->ToPlayer();
    if (!player || !sMythicPlus->GetRunByPlayer(player->GetGUID()))
        return;

    if (Creature* creature = attacker->ToCreature())
    {
        const float mult = sMythicPlus->OwnedDamage(creature->GetGUID());
        if (mult > 1.01f)
            damage = std::max<uint32>(1, static_cast<uint32>(damage * mult));
        uint8 level = 0;
        uint32 health = 0;
        if (sMythicPlus->OwnedLevel(creature->GetGUID(), level, health)
            && creature->GetHealthPct() < 30.0f
            && sMythicPlus->GetRunByPlayer(player->GetGUID())
            && sMythicPlus->GetRunByPlayer(player->GetGUID())->ModifierText.find("Raging") != std::string::npos)
            damage = std::max<uint32>(1, static_cast<uint32>(damage * 1.5f));
        return;
    }

    const uint32 cap = std::max<uint32>(1, static_cast<uint32>(player->GetMaxHealth() * 0.03f));
    if (damage > cap)
        damage = cap;
}
}

class mp_unit_script : public UnitScript
{
public:
    mp_unit_script() : UnitScript("mp_unit_script") {}

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* /*spellInfo*/) override
    {
        Scale(target, attacker, damage);
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* /*spellInfo*/) override
    {
        if (damage <= 0)
            return;
        uint32 amount = static_cast<uint32>(damage);
        Scale(target, attacker, amount);
        damage = static_cast<int32>(amount);
    }

    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        Scale(target, attacker, damage);
    }
};

void AddSC_mp_unit_script()
{
    new mp_unit_script();
}
