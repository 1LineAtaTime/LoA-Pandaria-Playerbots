/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/Seth/SethTriggers.cpp (#565, epic #569)

#include "SethTriggers.h"
#include "EncounterHelpers.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "SethShared.h"

using namespace SethShared;
using namespace EncounterHelpers;

bool TimeLostControllerDropsCharmingTotemTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SETH_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "time-lost controller");
}

bool SethekkProphetCastsFearTrigger::IsActive()
{
    if (bot->GetClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "sethekk prophet"))
        return false;

    return !AI_VALUE2(bool, "has totem", "tremor totem");
}

bool DarkweaverSythBossSummonsElementalsTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, SETH_MAP_ID))
        return false;

    Unit* syth = AI_VALUE2(Unit*, "find target", "darkweaver syth");
    return syth && syth->GetHealthPct() > BOSS_BURN_HEALTH_PCT;
}

bool AnzuEncounterHasTwoPhasesTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "anzu");
}

bool AnzuBirdSpiritsProvideBuffsTrigger::IsActive()
{
    return bot->GetClass() == CLASS_DRUID && PlayerBotSpec::IsHeal(bot) &&
        AI_VALUE2(Unit*, "find target", "anzu");
}

bool TalonKingIkissBossEngagedByTankTrigger::IsActive()
{
    if (!PlayerBotSpec::IsTank(bot))
        return false;

    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && ikiss->GetVictim() == bot && bot->IsWithinMeleeRange(ikiss);
}

bool TalonKingIkissRangedPrepareForArcaneExplosionTrigger::IsActive()
{
    if (!PlayerBotSpec::IsRanged(bot))
        return false;

    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && !ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE)) &&
        bot->IsWithinLOSInMap(ikiss);
}

bool TalonKingIkissBossCastingArcaneExplosionTrigger::IsActive()
{
    // Arcane Bubble is put up 1s before casting Arcane Explosion
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE));
}

bool TalonKingIkissBossOutOfLosTrigger::IsActive()
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && !ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE)) &&
        !bot->IsWithinLOSInMap(ikiss);
}
