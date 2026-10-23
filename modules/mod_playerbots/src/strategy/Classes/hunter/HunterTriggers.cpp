/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterTriggers.h"

#include "GenericTriggers.h"
#include "HunterActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Spell.h"
#include <set>

// Ported from AC mod-playerbots Ai/Class/Hunter/HunterTriggers.cpp (#149)

// Valid targets for "Improved Tracking".
bool NoTrackTrigger::IsActive()
{
    std::vector<std::string> track_list = {
        "track beasts",
        "track demons",
        "track dragonkin",
        "track elementals",
        "track giants",
        "track humanoids",
        "track undead",
    };

    for (auto& track : track_list)
    {
        if (botAI->HasAura(track, bot))
            return false;
    }

    return true;
}

bool HunterNoStingsActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return DebuffTrigger::IsActive() && target &&
           !botAI->HasAura("serpent sting", target, false, true) &&
           !botAI->HasAura("scorpid sting", target, false, true) &&
           !botAI->HasAura("viper sting", target, false, true);
}

bool HuntersPetDeadTrigger::IsActive()
{
    return AI_VALUE(bool, "pet dead") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HuntersPetMediumHealthTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    return pet && AI_VALUE2(uint8, "health", "pet target") < sPlayerbotAIConfig->mediumHealth &&
           !AI_VALUE2(bool, "dead", "pet target") && !AI_VALUE2(bool, "mounted", "self target");
}
// ---------------------------------------------------------------------------
// #357 slice 3: 1:1 port of AC mod-playerbots Ai/Class/Hunter/HunterTriggers.cpp.
// Applied 5.4.8 divergences:
//   * AC's `sPlayerbotAIConfig.mediumHealth` / `.lowMana` are `->` in this fork (PlayerbotAIConfig.h).
//   * AC's `BotState::BOT_STATE_COMBAT` is plain `BOT_STATE_COMBAT` here (SharedDefines.h), same as the
//     rest of the fork's strategy code.
//   * No TC_LOG_* here, so the TC_LOG printf-vs-fmt divergence does not apply to this file.
//   * AC includes "Player.h" for the `bot` member; this fork exposes it via the same headers.
// ---------------------------------------------------------------------------

bool KillCommandTrigger::IsActive() { return !botAI->HasAura("kill command", GetTarget()); }

bool BlackArrowTrigger::IsActive()
{
    if (botAI->HasStrategy("trap weave", BOT_STATE_COMBAT))
        return false;

    return DebuffTrigger::IsActive();
}

// AC HunterTriggers.cpp:31-48. "aspect of the hawk" here is an AURA on the target, not a trigger name
// (see the note at the bottom of HunterTriggers.h).
bool HunterAspectOfTheDragonhawkTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!SpellTrigger::IsActive())
        return false;

    if (botAI->HasAura("aspect of the hawk", target) ||
        botAI->HasAura("aspect of the dragonhawk", target))
        return false;

    if (botAI->HasAura("aspect of the viper", target))
        return AI_VALUE2(uint8, "mana", "self target") >= 60;

    return true;
}

bool HunterAspectOfTheViperTrigger::IsActive()
{
    if (botAI->HasStrategy("rnature", BOT_STATE_COMBAT) ||
        botAI->HasStrategy("rnature", BOT_STATE_NON_COMBAT) ||
        botAI->HasStrategy("bspeed", BOT_STATE_COMBAT) ||
        botAI->HasStrategy("bspeed", BOT_STATE_NON_COMBAT))
        return false;

    return BuffTrigger::IsActive() && AI_VALUE2(uint8, "mana", "self target") < (sPlayerbotAIConfig->lowMana / 2);
}

bool HunterAspectOfThePackTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("aspect of the cheetah", GetTarget());
}

bool HunterLowAmmoTrigger::IsActive()
{
    uint32 ammoCount = AI_VALUE2(uint32, "item count", "ammo");
    return bot->GetGroup() && ammoCount > 0 && ammoCount < 100;
}

bool HunterHasAmmoTrigger::IsActive() { return !AmmoCountTrigger::IsActive(); }

bool SwitchToRangedTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return botAI->HasStrategy("close", BOT_STATE_COMBAT) && target &&
           (target->GetVictim() != bot &&
            sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), 8.0f));
}

bool SwitchToMeleeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return botAI->HasStrategy("ranged", BOT_STATE_COMBAT) && target &&
           (target->GetVictim() == bot &&
            sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), 8.0f));
}

bool SerpentStingOnAttackerTrigger::IsActive()
{
    if (!DebuffOnAttackerTrigger::IsActive())
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    return !botAI->HasAura("scorpid sting", target, false, true) &&
           !botAI->HasAura("viper sting", target, false, true);
}

bool HuntersPetLowHealthTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    return pet && AI_VALUE2(uint8, "health", "pet target") < 40 && !AI_VALUE2(bool, "dead", "pet target") &&
           !AI_VALUE2(bool, "mounted", "self target");
}

// 5.4.8 divergence, reported not worked around: this fork has NO "pet happy" value - it is absent from
// strategy/ValueContext.h entirely (AC registers it; grep across the whole fork returns 0 hits), so
// AI_VALUE(bool, "pet happy") always yields the default false and this trigger is permanently TRUE
// while unmounted. Harmless today (no fork TriggerNode/NextAction consumes "pet not happy"), and the
// code is kept verbatim so it becomes correct the moment the value lands. value/* is outside this
// slice's ownership, so the value itself is filed as a gap rather than added here.
bool HunterPetNotHappy::IsActive()
{
    return !AI_VALUE(bool, "pet happy") && !AI_VALUE2(bool, "mounted", "self target");
}

const std::set<uint32> VolleyChannelCheckTrigger::VOLLEY_SPELL_IDS = {
    1510,   // Volley Rank 1
    14294,  // Volley Rank 2
    14295,  // Volley Rank 3
    27022,  // Volley Rank 4
    58431,  // Volley Rank 5
    58434   // Volley Rank 6
};

bool VolleyChannelCheckTrigger::IsActive()
{
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        spell && VOLLEY_SPELL_IDS.count(spell->m_spellInfo->Id))
    {
        uint8 attackerCount = AI_VALUE(uint8, "attacker count");
        return attackerCount < minEnemies;
    }

    return false;
}

// AC HunterTriggers.cpp:inline. Moved out of the header for the 5.4.8 reason given at the class.
bool LockAndLoadTrigger::IsActive()
{
    return botAI->HasAura("lock and load", bot);
}

// ===== #856 rotation lane 1 (epic #854) =====
bool HunterBindingShotRangeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !bot->IsInCombat())
        return false;

    float distance = bot->GetDistance(target);
    return distance >= 15.0f && distance <= 30.0f;
}

bool HunterDeterrenceHealthTrigger::IsActive()
{
    return bot->IsInCombat() && bot->IsAlive() && bot->GetHealthPct() <= 30.0f;
}

bool HunterDisengagePvpTrigger::IsActive()
{
    if (!bot->IsInCombat() || !AI_VALUE(bool, "in pvp instance"))
        return false;

    GuidVector attackers = AI_VALUE(GuidVector, "attackers");
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsPlayer() && unit->GetVictim() == bot && unit->IsAlive() && bot->GetDistance(unit) <= 5.0f)
            return true;
    }

    return false;
}

bool HunterSteadyFocusLowTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    // Rotation fix lane H: 53224 is the PASSIVE (permanent, duration -1; scripts/Spells/spell_hunter.cpp:1900) that counts two
    // consecutive Steady Shots, so the old test (53224 && duration <= 3000) was true on every tick and Steady Shot (5.84) always
    // out-prioritised Chimera/Aimed Shot (w74: T:steady focus active 20/24 ticks, Chimera/Aimed never cast). The haste BUFF it
    // casts is SPELL_HUNT_STEADY_FOCUS 53220 (spell_hunter.cpp:128/1921): refresh when it is missing or <= 3 s left.
    if (!bot->HasAura(53224))
        return false;  // not a talented/learned MM hunter
    Aura* buff = bot->GetAura(53220);
    return !buff || (buff->GetDuration() >= 0 && buff->GetDuration() <= 3000);
}

bool HunterLowHealthNoPvpTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("low health");
    return t && t->IsActive();
}

bool HunterMediumThreatNoPvpTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("medium threat");
    return t && t->IsActive();
}

// Rotation fix lane K (#868): see HunterTriggers.h. AC's "enemy too close for auto shot" outside a pvp instance / started duel.
bool HunterEnemyTooCloseTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("enemy too close for auto shot");
    return t && t->IsActive();
}

// Rotation fix lane K (#868): see HunterTriggers.h.
bool HunterDuelRetargetTrigger::IsActive()
{
    if (!bot->duel || !bot->duel->opponent || bot->duel->startTime == 0)
        return false;

    Unit* opponent = bot->duel->opponent;
    return opponent->IsAlive() && !AI_VALUE(Unit*, "current target");
}
