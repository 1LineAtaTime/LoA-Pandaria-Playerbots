/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#include "CataDmTriggers.h"
#include "CataDmShared.h"
#include "Playerbots.h"

bool DmBlossomLandingTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::BlossomLanding(botAI, bot, pos, radius);
}

bool DmBlossomAddTrigger::IsActive()
{
    return CataDm::BlossomAddFocus(botAI, bot) != nullptr;
}

bool DmStickyBombTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::StickyBombDanger(botAI, bot, pos, radius);
}

bool DmChestBombTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::ChestBombDanger(botAI, bot, pos, radius);
}

bool DmHelixFocusTrigger::IsActive()
{
    return CataDm::HelixFocus(botAI, bot) != nullptr;
}

bool DmHarvestTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::HarvestDanger(botAI, bot, pos, radius);
}

bool DmOverdriveTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::OverdriveDanger(botAI, bot, pos, radius);
}

bool DmVaporTrigger::IsActive()
{
    return CataDm::VaporFocus(botAI, bot) != nullptr;
}

bool DmVanessaAddTrigger::IsActive()
{
    return CataDm::VanessaAddFocus(botAI, bot) != nullptr;
}

// ---- #681 / #826 ----
bool DmFireWallTrigger::IsActive()
{
    return CataDm::FireWallDanger(bot);
}

bool DmMoltenSlagTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::MoltenSlagDanger(botAI, bot, pos, radius);
}

bool DmSlagAddTrigger::IsActive()
{
    return CataDm::MoltenSlagFocus(botAI, bot) != nullptr;
}

bool DmReaperFocusTrigger::IsActive()
{
    return CataDm::ReaperTankFocus(botAI, bot) != nullptr;
}

bool DmVanessaBlazeTrigger::IsActive()
{
    return (CataDm::VanessaRope(bot) && CataDm::VanessaFiresOnDeck(bot)) || CataDm::VanessaBlazeDanger(bot);
}

bool DmPowderKegTrigger::IsActive()
{
    Position pos;
    float radius = 0.0f;
    return CataDm::PowderKegDanger(botAI, bot, pos, radius);
}

bool DmSteamValveTrigger::IsActive()
{
    return CataDm::SteamValveToClick(bot) != nullptr;
}

bool DmNightmareHazardTrigger::IsActive()
{
    return CataDm::GroundHazardDanger(bot);
}

bool DmHarringtonTrigger::IsActive()
{
    return CataDm::HarringtonFocus(botAI, bot) != nullptr;
}
