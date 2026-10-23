/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#include "CataBcTriggers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

bool CataBcSkullcrackerTrigger::IsActive()
{
    return CataBc::SkullcrackerDanger(bot);
}

bool CataBcChainsTrigger::IsActive()
{
    return bot->IsInCombat() && CataBc::IsChained(bot) && CataBc::GetChains(bot) != nullptr;
}

bool CataBcQuakeTrigger::IsActive()
{
    return bot->HasAura(CataBc::SPELL_QUAKE);
}

bool CataBcLavaDroolTrigger::IsActive()
{
    float cx = 0.0f;
    float cy = 0.0f;
    return CataBc::GetLavaDroolDanger(bot, cx, cy);
}

bool CataBcEvolutionTrigger::IsActive()
{
    return CataBc::EvolutionDanger(bot);
}

bool CataBcInterruptTrigger::IsActive()
{
    return bot->IsInCombat() && CataBc::GetInterruptTarget(bot) != nullptr;
}

// Only the main tank that Karsh is actually attacking; every other bot ignores the pillar
bool CataBcKarshPillarTrigger::IsActive()
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsTank(bot))
        return false;

    Creature* karsh = CataBc::GetKarsh(bot);
    return karsh && karsh->GetVictim() == bot;
}
