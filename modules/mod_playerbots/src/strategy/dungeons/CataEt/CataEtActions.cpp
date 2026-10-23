/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - actions (issue #671, epic #373).

#include "CataEtActions.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Timer.h"
#include <algorithm>

// Same call as MgT's escape move (MgTActions.cpp MgTEscapeAction::Execute): combat priority, short delay.
bool CataEtMovementAction::StepTo(float x, float y, float z)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataEtMovementAction::LeaveHazards(std::vector<EndTime::Hazard> const& hazards)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!EndTime::FindHazardEscape(bot, hazards, x, y, z))
        return false;

    return StepTo(x, y, z);
}

bool CataEtEscapeAction::Execute(Event /*event*/)
{
    std::vector<EndTime::Hazard> hazards;
    EndTime::CollectHazards(bot, _kind, hazards);
    if (_kind == EndTime::HazardKind::Moonlance || _kind == EndTime::HazardKind::EyeOfElune)
    {
        // w68 (#671): Tyrande's escapes stay out of the Eye orbits (CataEtShared.cpp FindTyrandeHazardEscape).
        float x = 0.0f, y = 0.0f, z = 0.0f;
        if (!EndTime::FindTyrandeHazardEscape(bot, hazards, x, y, z))
            return false;

        return StepTo(x, y, z);
    }

    return LeaveHazards(hazards);
}

bool CataEtBladeDodgeAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!EndTime::GetBladeEscape(bot, x, y, z))
        return false;

    return StepTo(x, y, z);
}

bool CataEtFlarecoreAction::Execute(Event /*event*/)
{
    Creature* ember = EndTime::GetFlarecore(bot);
    if (!ember)
        return false;

    if (EndTime::GetFlarecoreStomper(bot, ember) == bot)
    {
        // The script detonates the ember when a player is within 3 yd (boss_echo_of_jaina.cpp:338-343).
        if (bot->GetExactDist2d(ember) < 1.5f)
            return false;

        return StepTo(ember->GetPositionX(), ember->GetPositionY(), ember->GetPositionZ());
    }

    std::vector<EndTime::Hazard> hazards;
    hazards.push_back({ ember->GetPositionX(), ember->GetPositionY(), 8.0f, 10.0f });
    return LeaveHazards(hazards);
}

bool CataEtSafeSpotAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!EndTime::GetSylvanasSafeSpot(bot, x, y, z))
        return false;

    if (bot->GetExactDist2d(x, y) <= 2.5f)
        return false;

    return StepTo(x, y, z);
}

bool CataEtSylvanasInsideAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!EndTime::GetSylvanasInsideSpot(bot, x, y, z))
        return false;

    return StepTo(x, y, z);
}

// AC PoS TyrannusAction::RangedSpread (PoSActions.cpp:292-312): small increments so the course can be corrected.
bool CataEtSpreadAction::Execute(Event /*event*/)
{
    constexpr float SPREAD_RADIUS = 10.0f;

    std::vector<Player*> members;
    EndTime::CollectBotMembers(bot, members);

    Player* closest = nullptr;
    float closestDist = 8.0f;
    for (Player* member : members)
    {
        if (member == bot)
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist < closestDist)
        {
            closest = member;
            closestDist = dist;
        }
    }

    if (!closest)
        return false;

    float const moveDistance = std::min(3.0f, SPREAD_RADIUS - closestDist + 1.0f);
    return FleePosition(closest->GetPosition(), moveDistance, 250U);
}

bool CataEtMoonlightPoolAction::Execute(Event /*event*/)
{
    Creature* pool = EndTime::GetMoonlightPool(bot);
    if (!pool)
        return false;

    return StepTo(pool->GetPositionX(), pool->GetPositionY(), pool->GetPositionZ());
}

bool CataEtTyrandeRangedBandAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!EndTime::GetTyrandeRangedSpot(bot, x, y, z))
        return false;

    return StepTo(x, y, z);
}

bool CataEtFragmentAction::Execute(Event /*event*/)
{
    GameObject* go = EndTime::FindJainaFragment(bot);
    if (!go)
        return false;

    // w62 (#671, the #818 click trap): MoveTo(target, d) stops on the SURFACE distance, so the use test takes the same measure;
    // an exact-distance test can leave the bot parked just outside it, never using the fragment.
    if (!bot->IsWithinDistInMap(go, INTERACTION_DISTANCE - 1.0f))
        return MoveTo(go, 2.0f, MovementPriority::MOVEMENT_NORMAL);

    // GameObject::Use is what HandleGameObjectUseOpcode reaches (the BG tactics call it the same way,
    // BattleGroundTactics.cpp:4458); the fragment script sits in OnGossipHello, which Use() calls first (GameObject.cpp:1304-1307).
    go->Use(bot);
    return true;
}

// MoveTo of a swimming bot is a direct MovePoint (generatePath = !isSwimming, MovementActions.cpp:401), so it crosses the
// lava without pathfinding.
bool CataEtLavaAction::Execute(Event /*event*/)
{
    GameObject* platform = EndTime::FindIntactPlatform(bot);
    if (!platform)
        return false;

    return StepTo(platform->GetPositionX(), platform->GetPositionY(),
                  platform->GetPositionZ() + EndTime::ET_PLATFORM_TOP_OFFSET);
}

bool CataEtTotemAction::Execute(Event /*event*/)
{
    Creature* baine = EndTime::GetBaine(bot);
    if (!baine)
        return false;

    if (bot->HasAura(EndTime::ET_SPELL_THROW_TOTEM_AURA))
    {
        // Ground-targeted: "Place the circle at Baine's feet" (Wowpedia "Echo of Baine").
        return botAI->CastSpell(EndTime::ET_SPELL_THROW_TOTEM_BACK, baine->GetPositionX(), baine->GetPositionY(),
                                baine->GetPositionZ());
    }

    Creature* totem = EndTime::GetBaineTotem(bot);
    if (!totem)
        return false;

    // Same surface-distance pair as the other click actions (MopScholoActions.cpp:96-97, MopGssActions.cpp:107-108).
    if (!bot->IsWithinDistInMap(totem, INTERACTION_DISTANCE - 1.0f))
        return MoveTo(totem, 2.0f, MovementPriority::MOVEMENT_COMBAT);

    // Same click as the vehicle boarding (VehicleActions.cpp:106): spellclick 54434 -> 107837.
    return totem->HandleSpellClick(bot);
}

bool CataEtBreathAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!EndTime::GetBreathEscape(bot, x, y, z))
        return false;

    return StepTo(x, y, z);
}

bool CataEtHourglassAction::Execute(Event /*event*/)
{
    GameObject* go = bot->FindNearestGameObject(EndTime::ET_GO_HOURGLASS, 250.0f);
    if (!go)
        return false;

    if (bot->GetExactDist(go) > 6.0f)
        return StepTo(go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());

    uint32 const now = getMSTime();
    if (_lastUse && now - _lastUse < 4000)
        return false;

    _lastUse = now;

    // go_murozond_hourglass_of_time only implements OnReportUse (boss_murozond.cpp:553-567); the client reaches it through
    // CMSG_GAMEOBJ_REPORT_USE (SpellHandler.cpp:633-680), which is this call after its distance check.
    sScriptMgr->OnReportUse(bot, go);
    return true;
}

bool CataEtKillGhoulAction::Execute(Event /*event*/)
{
    std::vector<Creature*> ghouls;
    EndTime::CollectRisenGhouls(bot, ghouls);
    Creature* ghoul = EndTime::GetDesignatedGhoul(ghouls);
    if (!ghoul)
        return false;

    return Attack(ghoul);
}

bool CataEtInterruptAction::Execute(Event /*event*/)
{
    Unit* target = _getter(bot);
    if (!target)
        return false;

    return !EndTime::CastInterrupt(botAI, target).empty();
}
