/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#include "CataLctTriggers.h"
#include "CataLctShared.h"
#include "Playerbots.h"

bool LctLandMineTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataLct::LandMineDanger(botAI, bot, pos, radius);
}

bool LctShockwaveTrigger::IsActive()
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    return CataLct::ShockwaveDanger(botAI, bot, x, y, z);
}

bool LctBadIntentionsTrigger::IsActive()
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    return CataLct::ThrownOnWall(botAI, bot, x, y, z);
}

bool LctOffPlatformTrigger::IsActive()
{
    return CataLct::FellOffPlatform(botAI, bot);
}

bool LctDustFlailTrigger::IsActive()
{
    return CataLct::DustFlailBoss(botAI, bot) != nullptr;
}

bool LctCrocoliskTrigger::IsActive()
{
    return CataLct::CrocoliskFocus(botAI, bot) != nullptr;
}

bool LctHeavensFuryTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataLct::HeavensFuryDanger(botAI, bot, pos, radius);
}

bool LctSoulFragmentTrigger::IsActive()
{
    return CataLct::SoulFragmentFocus(botAI, bot) != nullptr;
}

bool LctHallowedGroundTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataLct::HallowedGroundDanger(botAI, bot, pos, radius);
}

bool LctHarbingerTrigger::IsActive()
{
    return CataLct::HarbingerFocus(botAI, bot) != nullptr;
}

bool LctBlazeFireTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataLct::BlazeFireDanger(botAI, bot, pos, radius);
}

bool LctSiamatStormTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataLct::SiamatStormDanger(botAI, bot, pos, radius);
}

bool LctSiamatServantTrigger::IsActive()
{
    return CataLct::ServantFocus(botAI, bot) != nullptr;
}

bool LctLightningChargeTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataLct::LightningChargeDanger(botAI, bot, pos, radius);
}
