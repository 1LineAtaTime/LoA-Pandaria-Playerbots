/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#include "CataDmActions.h"
#include "CataDmShared.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Random.h"
#include <cmath>

using namespace EncounterHelpers;

int DmAvoidAction::EscapeBombs()
{
    // Our own escape walk still running towards a spot that is still clear: let it finish. Any other forced walk is cut
    // (same wait / cut as MopGss/MopGssActions.cpp:34-43).
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
    {
        LastMovement& last = AI_VALUE(LastMovement&, "last movement");
        bool const ours = _escaping && std::hypot(last.lastMoveToX - _escX, last.lastMoveToY - _escY) < 0.5f;
        if (ours && CataDm::BombSpotSafe(bot, _escX, _escY))
            return 0;

        last.clear();
    }

    _escaping = false;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataDm::BombEscapeSpot(bot, x, y, z))
        return -1;

    // MOVEMENT_FORCED: a melee chase / reach move at MOVEMENT_COMBAT must not keep the bot on the bomb (MopGss w59 finding,
    // MopGssActions.cpp:27-29); FleePosition's own MoveTo arguments otherwise (MovementActions.cpp:1552-1553).
    if (!MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_FORCED))
        return -1;

    _escaping = true;
    _escX = x;
    _escY = y;
    return 1;
}

bool DmLeaveBlossomAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::BlossomLanding(botAI, bot, pos, radius))
        return false;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmKillBlossomAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::BlossomAddFocus(botAI, bot);
    if (!target)
        return false;

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataDm::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}

bool DmLeaveStickyBombAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::StickyBombDanger(botAI, bot, pos, radius))
        return false;

    // #667 w68: 3482 FAILED / 322 OK. FleePosition steps straight away from the one nearest bomb, at most once a second
    // (MovementActions.cpp:1538, "recently flee info"); a bomb dropped at the bot's feet always sends it due west (atan2(0,0) = 0),
    // and a bot without a target has no other angle - the tank stood 80 s against the west wall (-313.9,-520) with the trigger
    // on, every attack zeroed, and never tanked. One spot clear of all bombs first; FleePosition only as the fallback.
    int const escape = EscapeBombs();
    if (escape >= 0)
        return escape > 0;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmSpreadChestBombAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::ChestBombDanger(botAI, bot, pos, radius))
        return false;

    // #667 w68: the same escape as the sticky bombs, so leaving a carrier never walks onto a live bomb (and back)
    int const escape = EscapeBombs();
    if (escape >= 0)
        return escape > 0;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmAttackHelixAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::HelixFocus(botAI, bot);
    if (!target)
        return false;

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataDm::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}

bool DmLeaveHarvestAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::HarvestDanger(botAI, bot, pos, radius))
        return false;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmLeaveOverdriveAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::OverdriveDanger(botAI, bot, pos, radius))
        return false;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmKillVaporAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::VaporFocus(botAI, bot);
    if (!target)
        return false;

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataDm::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}

bool DmKillVanessaAddAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::VanessaAddFocus(botAI, bot);
    if (!target)
        return false;

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataDm::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}

// ---- #681 / #826 ----
bool DmDodgeFireWallAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataDm::FireWallSpot(bot, x, y, z))
        return false;

    // MOVEMENT_FORCED as the bomb escape (a melee chase at MOVEMENT_COMBAT must not hold the bot in the wall's path)
    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_FORCED);
}

bool DmAvoidMoltenSlagAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::MoltenSlagDanger(botAI, bot, pos, radius))
        return false;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmKillMoltenSlagAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::MoltenSlagFocus(botAI, bot);
    if (!target)
        return false;

    return Attack(target);
}

bool DmAttackReaperAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::ReaperTankFocus(botAI, bot);
    if (!target)
        return false;

    return Attack(target);
}

bool DmEscapeBlazeAction::Execute(Event /*event*/)
{
    // the rope: walk to it while the detonation fires are on the deck, take it when they blaze (CataDmShared.h VanessaRope)
    Creature* rope = CataDm::VanessaRope(bot);
    if (rope && CataDm::VanessaFiresOnDeck(bot))
    {
        if (bot->GetExactDist2d(rope) <= CataDm::ROPE_CLICK_RANGE)
        {
            if (!CataDm::VanessaBlazeActive(bot) && !CataDm::VanessaBlazeDanger(bot))
                return false;   // at the rope; the fires are not burning yet

            bot->StopMoving();
            // 5.4.8 Unit::HandleSpellClick (TOC/TOCActions.cpp:257, MopGssActions.cpp:131): npc_spellclick_spells 49550 -> 46598
            // Ride Vehicle; the rope's script (npc_dm_vanessa_rope) swings the rider off the ship and back
            // (the seat is taken a tick later: VanessaRope notes the ride when it sees the bot on the rope)
            rope->HandleSpellClick(bot);
            return true;
        }

        return MoveTo(bot->GetMapId(), rope->GetPositionX(), rope->GetPositionY(), rope->GetPositionZ(), false, false, true, false,
                      MovementPriority::MOVEMENT_FORCED);
    }

    // no rope left (or none yet): one spot clear of every fire
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataDm::GroundHazardEscapeSpot(bot, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_FORCED);
}

bool DmLeavePowderKegAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataDm::PowderKegDanger(botAI, bot, pos, radius))
        return false;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool DmClickSteamValveAction::Execute(Event /*event*/)
{
    Creature* valve = CataDm::SteamValveToClick(bot);
    if (!valve)
        return false;

    // the valve's script (npc_dm_steam_valve::OnSpellClick) makes it unselectable once counted
    valve->HandleSpellClick(bot);
    return valve->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool DmEscapeNightmareHazardAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataDm::GroundHazardEscapeSpot(bot, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_FORCED);
}

bool DmSaveHarringtonAction::Execute(Event /*event*/)
{
    Unit* target = CataDm::HarringtonFocus(botAI, bot);
    if (!target)
        return false;

    // #826 w107: James is up on the ship, out of reach - walk up the ramp first (Attack() refuses a target out of sight,
    // AttackActions.cpp:115-122). MOVEMENT_FORCED as the hazard escapes; a walk in progress to the same spot keeps the tick, so the
    // "follow" of the non-combat engine does not turn the bot back to its master on Erik's dock.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (CataDm::JamesApproachSpot(bot, x, y, z))
    {
        if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
        {
            LastMovement& last = AI_VALUE(LastMovement&, "last movement");
            if (std::hypot(last.lastMoveToX - x, last.lastMoveToY - y) < 1.0f)
                return true;
        }

        return MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_FORCED);
    }

    if (PlayerBotSpec::IsHeal(bot))
        return false;   // a healer only walks up (HarringtonFocus)

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataDm::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}
