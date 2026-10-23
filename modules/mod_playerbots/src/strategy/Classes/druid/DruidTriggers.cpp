/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DruidTriggers.h"

#include "DynamicObject.h"
#include "Map.h"
#include "Player.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Spell.h"

// #521 w13: see DruidTriggers.h - the 5.4.8 Eclipse auras have no duration, so check them without the duration filter.
bool EclipseSolarTrigger::IsActive() { return botAI->HasAura(getName(), GetTarget(), false, false, -1, false); }

bool EclipseLunarTrigger::IsActive() { return botAI->HasAura(getName(), GetTarget(), false, false, -1, false); }

// 67484 Eclipse Marker (lunar direction), spell_druid.cpp SPELL_DRUID_ECLIPSE_MARKER_LUNAR.
bool EclipseLunarDirectionTrigger::IsActive() { return bot->HasAura(67484); }

// 1:1 port of AC Ai/Class/Druid/DruidTriggers.cpp (WotLK mod-playerbots).
//
// 5.4.8 divergences applied here:
//  - ServerFacade::instance() returns a POINTER in this fork (Utils/ServerFacade.h:21, `sServerFacade` is
//    the #define for it), so every call is `sServerFacade->Method(` - the AC `ServerFacade::instance().X`
//    form is C2228 here.
//  - Every IsActive() body is here, not inline in the header: PlayerbotAI::HasAura(std::string, Unit*)
//    cannot bind a `bot` (Player*) from a header TU here (C2665).
//  - aquatic form reads `bot->GetLiquidStatus()` (Object.h:471, a ZLiquidStatus bitmask) rather than AC's
//    `bot->GetLiquidData().Status`; the constant is a bit flag, so it is tested with & - the same test
//    AC's `== LIQUID_MAP_UNDER_WATER` performs (Map.h:143).

bool MarkOfTheWildTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("gift of the wild", GetTarget());
}

bool ThornsOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget());
}

bool ThornsTrigger::IsActive() { return BuffTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget()); }

bool NaturesSwiftnessTrigger::IsActive()
{
    uint32 const spellId = AI_VALUE2(uint32, "spell id", "nature's swiftness");
    return spellId && bot->HasSpell(spellId) && BuffTrigger::IsActive();
}

// Wave-75 fix J (#860): see ProwlActiveTrigger in DruidTriggers.h. Prowl is a stealth aura with duration -1.
bool ProwlActiveTrigger::IsActive() { return botAI->HasAura("prowl", bot); }

// Wave-75 fix J (#868): see DruidNotBehindTargetTrigger in DruidTriggers.h.
bool DruidNotBehindTargetTrigger::IsActive()
{
    if (!IsNotBehindTargetTrigger::IsActive())
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return !(target && target->IsPlayer() && AI_VALUE(bool, "in pvp instance"));
}

bool NaturesSwiftnessActiveTrigger::IsActive() { return botAI->HasAura("nature's swiftness", bot); }

bool RakeTrigger::IsActive() { return !botAI->HasAura("prowl", bot) && DebuffTrigger::IsActive(); }

bool InsectSwarmOnAttackerTrigger::IsActive() { return BuffTrigger::IsActive(); }

bool MoonfireOnAttackerTrigger::IsActive() { return BuffTrigger::IsActive(); }

// #624 (AC BalanceDruidStrategy.cpp:193, DruidTriggers.h FaerieFireTrigger): 5.x Faerie Fire 770's purpose is the Weakened
// Armor 113746 debuff (3 stacks; Wowhead mop-classic 770 + 113746, 2026-10-05), shared with Expose Armor/Sunder/Devastate.
// Do not recast it while any caster's Weakened Armor already sits at 3 stacks.
bool FaerieFireTrigger::IsActive()
{
    Unit* target = GetTarget();
    return DebuffTrigger::IsActive() && !botAI->HasAura("weakened armor", target, true, false, -1, true);
}

bool FaerieFireFeralTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    // Bear: every cast generates immediate threat/damage for free - spam it
    if (botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr))
    {
        Unit* target = GetTarget();
        return target && target->IsAlive() && target->IsInWorld();
    }

    if (!botAI->HasAura("cat form", bot))
        return false;

    if (botAI->HasAura("prowl", bot))
        return false;

    // Cat with Omen of Clarity: spam to fish for Clearcasting procs
    if (bot->HasAura(AURA_OMEN_OF_CLARITY))
    {
        Unit* target = GetTarget();
        return target && target->IsAlive() && target->IsInWorld();
    }

    // Cat without Omen of Clarity: apply as a normal debuff, don't reapply
    return DebuffTrigger::IsActive();
}

bool BearFormTrigger::IsActive() { return !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr); }

bool TreeFormTrigger::IsActive()
{
    constexpr uint32 SPELL_TREE_OF_LIFE = 33891;
    return !bot->HasAura(SPELL_TREE_OF_LIFE);
}

bool CatFormTrigger::IsActive() { return !botAI->HasAura("cat form", bot); }

bool AquaticFormTrigger::IsActive()
{
    return !bot->IsInCombat() && !botAI->HasAura("aquatic form", bot) &&
           (bot->GetLiquidStatus() & LIQUID_MAP_UNDER_WATER);
}

bool MangleBearTrigger::IsActive()
{
    if (!bot->IsInCombat() || !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr))
        return false;
    Unit* target = GetTarget();
    return target && target->IsAlive() && target->IsInWorld();
}

bool LacerateTrigger::IsActive()
{
    if (!bot->IsInCombat() || !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr))
        return false;

    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    Aura* lacerate = botAI->GetAura("lacerate", target, false, false);
    if (!lacerate)
        return true;

    if (lacerate->GetStackAmount() < 5)
        return true;

    return lacerate->GetDuration() <= 6000;
}

bool DemoralizeRoarTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && !botAI->HasAura("curse of weakness", GetTarget(), false, false) &&
           !botAI->HasAura("demoralizing shout", GetTarget(), false, false) &&
           !botAI->HasAura("vindication", GetTarget(), false, false);
}

bool MangleCatTrigger::IsActive()
{
    if (botAI->HasAura("prowl", bot))
        return false;
    // #384: AC's second check (HasAura "mangle (bear)") is the same debuff under the 3.3.5 bear name; 5.x has one
    // Mangle, which DebuffTrigger::IsActive() already tests by its own name ("mangle"), so it is dropped here.
    return DebuffTrigger::IsActive() && !botAI->HasAura("trauma", GetTarget(), false, false, -1, true);
}

// #384 (AC DruidTriggers.cpp DemoralizeRoarTrigger::IsActive, retargeted at Thrash - see DruidTriggers.h).
// AC skips the roar when a curse of weakness / demoralizing shout / vindication already demoralizes the target;
// 5.x puts all of those under the single "Weakened Blows" debuff (Spell.dbc name; Wowhead mop-classic 106830).
bool ThrashTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && !botAI->HasAura("weakened blows", GetTarget(), false, false, -1, true);
}

bool EntanglingRootsKiteTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && AI_VALUE(uint8, "attacker count") < 3 && !GetTarget()->GetPower(POWER_MANA);
}

bool FerociousBiteTimeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    uint8 cp = AI_VALUE2(uint8, "combo", "current target");
    if (cp < 5)
        return false;

    // Rotation fix lane A (#860), MoP rotation row 10302 (spec 103, prio group 2/2, "combo points 5-5" only; Savage Roar
    // 10300 refreshes at <= 6 s, Rip 10301 at <= 2 s): AC's 10 s guards (Savage Roar / Rip) are narrowed to those MoP
    // refresh windows. w73: with 10 s the 5-CP window with both buffs healthy was too short, and the one window that opened
    // ended in NOT_READY/NO_POWER, so Ferocious Bite never landed (checkcast 70 x3, power 88 x1).
    Aura* roar = botAI->GetAura("savage roar", bot);
    if (roar && roar->GetDuration() <= 6000)
        return false;

    Aura* rip = botAI->GetAura("rip", target, true);
    if (rip && rip->GetDuration() <= 2000)
        return false;

    return true;
}

bool FerociousBiteExecuteTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    // 5.4.8 divergence: AC calls botAI->HasSpell("ferocious bite") (a spell-name overload). This core only
    // has Player::HasSpell(uint32) (src/server/game/Entities/Player/Player.h:2017), so the name is
    // resolved through the "spell id" value - the same idiom PriestTriggers.cpp:37 uses. 0 = not known.
    uint32 const spellId = AI_VALUE2(uint32, "spell id", "ferocious bite");
    if (!spellId || !bot->HasSpell(spellId))
        return false;

    if (AI_VALUE2(uint8, "combo", "current target") < 1)
        return false;

    if (target->GetHealthPct() >= 25.0f)
        return false;

    if (target->GetHealth() >= 20000)
        return false;

    return true;
}

const std::set<uint32> HurricaneChannelCheckTrigger::HURRICANE_SPELL_IDS = {
    16914,  // Hurricane Rank 1
    17401,  // Hurricane Rank 2
    17402,  // Hurricane Rank 3
    27012,  // Hurricane Rank 4
    48467   // Hurricane Rank 5
};

bool HurricaneChannelCheckTrigger::IsActive()
{
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        if (!HURRICANE_SPELL_IDS.count(spell->m_spellInfo->Id))
            return false;

        // Find this bot's own Hurricane DynamicObject
        DynamicObject* dynObj = nullptr;
        for (uint32 spellId : HURRICANE_SPELL_IDS)
        {
            dynObj = bot->GetDynObject(spellId);
            if (dynObj)
                break;
        }

        if (!dynObj)
            return false;

        // Count attackers actually inside the Hurricane AoE
        float radius = dynObj->GetRadius();
        GuidVector attackers = AI_VALUE(GuidVector, "attackers");
        uint32 count = 0;
        for (ObjectGuid const& guid : attackers)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
            if (unit->GetDistance(dynObj->GetPosition()) <= radius)
                count++;
        }

        return count < minEnemies;
    }

    return false;
}

bool NoHealerDpsStrategyTrigger::IsActive() { return !botAI->HasStrategy("healer dps", BOT_STATE_COMBAT); }

bool ProwlTrigger::IsActive()
{
    if (botAI->HasAura("prowl", bot) || bot->IsInCombat())
        return false;

    // 5.4.8 divergence: Player::HasSpell(uint32) only (Player.h:2017) - see FerociousBiteExecuteTrigger.
    uint32 const prowlId = AI_VALUE2(uint32, "spell id", "prowl");
    if (!prowlId || !bot->HasSpell(prowlId))
        return false;

    if (bot->HasSpellCooldown(prowlId))
        return false;

    float distance = 30.f;

    Unit* target = AI_VALUE(Unit*, "enemy player target");
    if (target && !target->IsInWorld())
        return false;
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

// Rotation lane 5 (#860): MoP rotation row 10310/10311 - Predatory Swiftness (69369) makes Healing Touch instant.
bool PredatorsSwiftnessHealTrigger::IsActive()
{
    return botAI->HasAura("predatory swiftness", bot) && AI_VALUE2(uint8, "health", "self target") <= maxHealth;
}

// Rotation lane 5 (#860): MoP rotation row 10505 (Ironbark, tank health <= 40%).
bool IronbarkTrigger::IsActive()
{
    Unit* tank = AI_VALUE(Unit*, "main tank");
    if (!tank || !tank->IsAlive() || tank->GetHealthPct() > 40.0f)
        return false;

    return !botAI->HasAura("ironbark", tank);
}
