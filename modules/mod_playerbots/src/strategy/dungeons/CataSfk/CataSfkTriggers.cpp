/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#include "CataSfkTriggers.h"
#include "CataSfkShared.h"
#include "Playerbots.h"

bool SfkPainAndSufferingTrigger::IsActive()
{
    return CataSfk::AshburyInterruptTarget(botAI, bot) != nullptr;
}

bool SfkWorgenSpiritTrigger::IsActive()
{
    return CataSfk::WorgenSpiritFocus(botAI, bot) != nullptr;
}

bool SfkWorgenChannelTrigger::IsActive()
{
    return CataSfk::WorgenInterruptTarget(botAI, bot) != nullptr;
}

bool SfkDesecrationTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataSfk::DesecrationDanger(botAI, bot, pos, radius);
}

bool SfkUnholyEmpowermentTrigger::IsActive()
{
    return CataSfk::EmpowermentInterruptTarget(botAI, bot) != nullptr;
}

bool SfkShieldConeTrigger::IsActive()
{
    return CataSfk::ShieldConeBoss(botAI, bot) != nullptr;
}

bool SfkPoisonMixtureTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataSfk::PoisonMixtureDanger(botAI, bot, pos, radius);
}

bool SfkKeepMovingTrigger::IsActive()
{
    return CataSfk::KeepMovingDanger(botAI, bot);
}

bool SfkToxicCoagulantTrigger::IsActive()
{
    return CataSfk::CoagulantJump(botAI, bot);
}

bool SfkCursedBulletsTrigger::IsActive()
{
    return CataSfk::CursedBulletsInterruptTarget(botAI, bot) != nullptr;
}

bool SfkPistolBarrageTrigger::IsActive()
{
    return CataSfk::BarrageConeBoss(botAI, bot) != nullptr;
}

bool SfkBloodthirstyGhoulTrigger::IsActive()
{
    return CataSfk::GhoulFocus(botAI, bot) != nullptr;
}
