/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#include "RogueTriggers.h"

#include "GenericTriggers.h"
#include "Item.h"
#include "Playerbots.h"
#include "ServerFacade.h"

namespace
{
constexpr uint32 SPELL_STEALTH = 1784;
constexpr uint32 SPELL_SPRINT_RANK_1 = 2983;

// 5.4.8 divergence: AC calls botAI->HasSpell("sap") (spell-name overload). This core only has
// Player::HasSpell(uint32) (src/server/game/Entities/Player/Player.h:2017), so the name is resolved
// through the "spell id" value - the idiom PriestTriggers.cpp:37 already uses. Same test, same result:
// 0 = spell not known.
//
// NOTE: this CANNOT be a free helper - AI_VALUE2 expands to `context->GetValue<...>()` and `context` is a
// member of AiObject, so the lookup must happen inside the trigger class. Inlined at its single call site
// (SapTrigger::IsPossible) rather than passed around.
}  // namespace

// bool AdrenalineRushTrigger::IsPossible()
// {
//     return !botAI->HasAura("stealth", bot);
// }

bool UnstealthTrigger::IsActive()
{
    if (!botAI->HasAura("stealth", bot))
        return false;

    if (!AI_VALUE2(bool, "moving", "self target"))
        return false;

    if (!AI_VALUE(uint8, "attacker count"))
        return true;

    return botAI->GetMaster() && sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "group leader"), 10.0f) &&
           AI_VALUE2(bool, "moving", "group leader");
}

bool StealthTrigger::IsActive()
{
    if (bot->HasAura(SPELL_STEALTH) || bot->IsInCombat() || bot->HasSpellCooldown(SPELL_STEALTH))
        return false;

    // (Stealth, prio 5/1): "NOT carrying a PvP objective (flag/orb)".
    if (AI_VALUE(bool, "carrying objective"))
        return false;

    float distance = 30.f;

    Unit* target = AI_VALUE(Unit*, "enemy player target");
    if (target && !target->IsInWorld())
    {
        return false;
    }
    if (!target)
        target = AI_VALUE(Unit*, "grind target");

    if (!target)
        target = AI_VALUE(Unit*, "dps target");

    if (!target)
        return false;

    if (target && target->GetVictim())
        distance -= 10;

    if (target->isMoving() && target->GetVictim())
        distance -= 10;

    if (bot->InBattleground())
        distance += 15;

    if (bot->InArena())
        distance += 15;

    return target && sServerFacade->GetDistance2d(bot, target) < distance;
}

bool SapTrigger::IsPossible()
{
    return bot->GetLevel() > 10 && AI_VALUE2(uint32, "spell id", "sap") != 0 && !bot->IsInCombat();
}

bool SprintTrigger::IsPossible()
{
    return bot->HasSpell(SPELL_SPRINT_RANK_1);
}

bool SprintTrigger::IsActive()
{
    if (bot->HasSpellCooldown(SPELL_SPRINT_RANK_1))
        return false;

    float distance = botAI->GetMaster() ? 45.0f : 35.0f;
    if (botAI->HasAura("stealth", bot))
        distance -= 10;

    bool targeted = false;

    Unit* dps = AI_VALUE(Unit*, "dps target");
    Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target");

    if (enemyPlayer && !enemyPlayer->IsInWorld())
    {
        return false;
    }
    if (dps)
        targeted = (dps == AI_VALUE(Unit*, "current target"));

    if (enemyPlayer && !targeted)
        targeted = (enemyPlayer == AI_VALUE(Unit*, "current target"));

    if (!targeted)
        return false;

    if ((dps && dps->IsInCombat()) || enemyPlayer)
        distance -= 10;

    return AI_VALUE2(bool, "moving", "self target") && (AI_VALUE2(bool, "moving", "dps target") || AI_VALUE2(bool, "moving", "enemy player target")) &&
           targeted && (sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "dps target"), distance) ||
                        sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "enemy player target"), distance));
}

// ---- Rotation port (epic #854, issue #859)

bool RevealingStrikeTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", "revealing strike") != 0 && DebuffTrigger::IsActive();
}

bool HemorrhageTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", "hemorrhage") != 0 && DebuffTrigger::IsActive();
}

bool ShadowstepTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "shadowstep");
    if (!spellId || bot->HasSpellCooldown(spellId) || botAI->HasAura("stealth", bot))
        return false;

    if (!AI_VALUE(Unit*, "current target"))
        return false;

    float distance = AI_VALUE2(float, "distance", "current target");
    return distance >= 8.0f && distance <= 25.0f;
}

bool MarkedForDeathTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "marked for death");
    if (!spellId || bot->HasSpellCooldown(spellId) || botAI->HasAura("stealth", bot))
        return false;

    return AI_VALUE(Unit*, "current target") && AI_VALUE2(uint8, "combo", "current target") < 2;
}

bool VendettaTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "vendetta");
    if (!spellId || bot->HasSpellCooldown(spellId) || botAI->HasAura("stealth", bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !botAI->HasAura("vendetta", target, false, true);
}

bool RecuperateTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "recuperate");
    if (!spellId)
        return false;

    return AI_VALUE2(uint8, "health", "self target") <= 30 && AI_VALUE2(uint8, "combo", "current target") >= 5 &&
           !botAI->HasAura("recuperate", bot, false, true);
}

bool PreparationTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "preparation");
    if (!spellId || bot->HasSpellCooldown(spellId) || !bot->IsInCombat())
        return false;

    for (char const* name : {"vanish", "sprint", "shadowstep"})
    {
        uint32 id = AI_VALUE2(uint32, "spell id", name);
        if (id && bot->HasSpellCooldown(id))
            return true;
    }

    return false;
}

bool ShadowBladesTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", "shadow blades") != 0 && bot->IsInCombat() && BuffTrigger::IsActive();
}

bool SliceAndDiceTrigger::IsActive()
{
    return BuffTrigger::IsActive() && AI_VALUE2(uint8, "combo", "current target") >= 1;
}

bool ExposeArmorTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    // #624 (AC RogueTriggers.cpp:115-120): 5.4.8 Expose Armor 8647 applies Weakened Armor 113746 (SpellEffect.dbc effect 64
    // TRIGGER_SPELL; Wowhead mop-classic 113746, 2026-10-05: -4% armor, stacks 3x, shared with Sunder Armor/Devastate/Faerie
    // Fire), never an aura named "expose armor"/"sunder armor", so the unchanged AC check re-cast it every GCD (wave 11).
    // Satisfied by Weakened Armor at 3 stacks from any caster.
    return DebuffTrigger::IsActive() && !botAI->HasAura("weakened armor", target, true, false, -1, true) && AI_VALUE2(uint8, "combo", "current target") <= 3;
}

bool MainHandWeaponNoEnchantTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return false;

    return true;
}

bool OffHandWeaponNoEnchantTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return false;

    return true;
}
