/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataGbActions.h"
#include "CataGbShared.h"
#include "EncounterHelpers.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

bool GbMoveAction::GoTo(float x, float y, MovementPriority priority)
{
    return MoveTo(bot->GetMapId(), x, y, CataGb::GroundZ(bot, x, y), false, false, false, true, priority);
}

bool GbUmbrissGroundSiegeAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::GroundSiegeTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbUmbrissBlitzAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::BlitzTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbThrongusPhalanxAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::PhalanxTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbThrongusCaveInAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::CaveInTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbThrongusFirePatchAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::FirePatchTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbValionaBreathAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::ValionaBreathTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbDrahgaPortalAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::FlamePortalTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbDrahgaKiteAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::SpiritKiteTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbDrahgaSpiritAction::Execute(Event /*event*/)
{
    Creature* spirit = CataGb::FlamingSpirit(bot);
    if (!spirit)
        return false;

    // Marking only: the bot's own target choice (dps assist) does the attacking, as in AC's FoS action.
    EncounterHelpers::MarkTargetWithSkull(bot, spirit);
    return false;
}

bool GbDrahgaSeepingTwilightAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::SeepingTwilightTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbErudaxGaleAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::ShadowGaleTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbErudaxShadowsAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataGb::BindingShadowsTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool GbErudaxCorruptorAction::Execute(Event /*event*/)
{
    Creature* corruptor = CataGb::FacelessCorruptor(bot);
    if (!corruptor)
        return false;

    EncounterHelpers::MarkTargetWithSkull(bot, corruptor);
    return false;
}
