/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DKTriggers.h"

#include <string>

#include "GenericTriggers.h"
#include "Playerbots.h"
#include "SharedDefines.h"

bool DeathAndDecayCooldownTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    if (!spellId)
        return true;

    return bot->GetSpellCooldownDelay(spellId) >= 2000;
}

bool NoRuneTrigger::IsActive()
{
    for (uint8 i = 0; i < MAX_RUNES; ++i)
    {
        if (!bot->HasRuneCooldown(i))
            return false;
    }
    return true;
}

namespace
{
    // Remaining "ms-equivalent" before rune index i is ready again; 0 if it already is.
    uint32 RuneRemaining(Player* bot, uint8 index)
    {
        return RUNE_READINESS_FULL - bot->GetRune(index).Readiness;
    }
}

bool HighBloodRuneTrigger::IsActive() { return RuneRemaining(bot, 0) <= 2000 && RuneRemaining(bot, 1) <= 2000; }

bool HighFrostRuneTrigger::IsActive() { return RuneRemaining(bot, 4) <= 2000 && RuneRemaining(bot, 5) <= 2000; }

bool HighUnholyRuneTrigger::IsActive() { return RuneRemaining(bot, 2) <= 2000 && RuneRemaining(bot, 3) <= 2000; }


bool DKPresenceTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("blood presence", target) && !botAI->HasAura("unholy presence", target) &&
           !botAI->HasAura("frost presence", target);
}

// 5.4.8: inert - see the PestilenceGlyphTrigger comment in DKTriggers.h. AC returned true when the
// WotLK Glyph of Pestilence (63334) was up and a disease had <= 3s left; that glyph is gone in MoP.
bool PestilenceGlyphTrigger::IsActive()
{
    return false;
}

// ---------------------------------------------------------------------------------------------------------
// rot-lane-2 (#857): MoP rotation triggers (declarations in DKTriggers.h)
// ---------------------------------------------------------------------------------------------------------
namespace
{
    constexpr uint32 SPELL_BLOOD_PLAGUE = 55078;
    constexpr uint32 SPELL_FROST_FEVER = 55095;
    constexpr uint32 SPELL_CHAINS_OF_ICE = 45524;
    constexpr uint32 SPELL_KILLING_MACHINE = 51124;
    constexpr uint32 SPELL_SUDDEN_DOOM = 81340;
    // rot-fix-F: 81136 is the Blood passive learned with the spec (SpecializationSpells.dbc row 3319, spec 250), so
    // HasAura(81136) was always true and "dk blood boil" fired every tick, starving Soul Reaper / Heart Strike.
    // The free-Blood-Boil proc buff is 81141 (also named "Crimson Scourge", spell_dk.cpp:3056 script is on 81136).
    constexpr uint32 SPELL_CRIMSON_SCOURGE = 81141;
    constexpr uint32 SPELL_OUTBREAK = 77575;
    constexpr uint32 SPELL_FESTERING_STRIKE = 85948;
}

int32 DKCurrentTargetTrigger::OwnAuraLeft(Unit* target, uint32 spellId) const
{
    Aura* aura = target->GetAura(spellId, bot->GetGUID());
    return aura ? aura->GetDuration() : 0;
}

bool DKOutbreakTrigger::IsActive()
{
    if (!bot->HasSpell(SPELL_OUTBREAK))
        return false;
    Unit* target = GetTarget();
    if (!target || !target->IsAlive())
        return false;
    return !target->HasAura(SPELL_BLOOD_PLAGUE, bot->GetGUID()) || !target->HasAura(SPELL_FROST_FEVER, bot->GetGUID());
}

bool DKDiseaseExpiringTrigger::IsActive()
{
    if (!bot->HasSpell(SPELL_FESTERING_STRIKE))
        return false;
    Unit* target = GetTarget();
    if (!target || !target->IsAlive())
        return false;
    int32 bp = OwnAuraLeft(target, SPELL_BLOOD_PLAGUE);
    int32 ff = OwnAuraLeft(target, SPELL_FROST_FEVER);
    return (bp > 0 && bp <= withinMs) || (ff > 0 && ff <= withinMs);
}

bool DKBloodBoilTrigger::IsActive()
{
    if (bot->HasAura(SPELL_CRIMSON_SCOURGE))
        return true;
    Unit* target = GetTarget();
    if (!target || !target->IsAlive())
        return false;
    int32 bp = OwnAuraLeft(target, SPELL_BLOOD_PLAGUE);
    int32 ff = OwnAuraLeft(target, SPELL_FROST_FEVER);
    return (bp > 0 && bp <= 3000) || (ff > 0 && ff <= 3000);
}

bool DKFrostStrikeProcTrigger::IsActive()
{
    return bot->GetPowerPct(POWER_RUNIC_POWER) >= 89.0f || bot->HasAura(SPELL_KILLING_MACHINE);
}

bool DKDeathCoilProcTrigger::IsActive()
{
    return bot->GetPowerPct(POWER_RUNIC_POWER) >= 90.0f || bot->HasAura(SPELL_SUDDEN_DOOM);
}

bool DKChainsOfIcePlayerTrigger::IsActive()
{
    if (!AI_VALUE(bool, "in pvp instance"))
        return false;
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || target->GetTypeId() != TYPEID_PLAYER)
        return false;
    if (target->HasAura(SPELL_CHAINS_OF_ICE, bot->GetGUID()))
        return false;
    return bot->GetDistance(target) >= 6.0f;
}

bool DKPvpEnemyCasterTrigger::IsActive()
{
    return AI_VALUE(bool, "in pvp instance") && AI_VALUE(Unit*, "enemy caster target");
}
