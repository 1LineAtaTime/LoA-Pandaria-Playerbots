/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) actions, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSActions.cpp and
// MgTActions.cpp (escape step with MovementPriority::MOVEMENT_COMBAT, focus attack).

#include "CataZgActions.h"
#include "Creature.h"
#include "GameObject.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectDefines.h"
#include "Playerbots.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <vector>

namespace Zg = CataZulGurub;

bool CataZgEscapeAction::Execute(Event /*event*/)
{
    std::vector<Zg::Hazard> hazards;
    Zg::CollectHazards(bot, _kind, hazards);
    if (hazards.empty())
        return false;

    // #679 w106: the mage walked an Acrid Pools escape (up to 24 yd, ~3.5 s of IsWaitingForLastMove) through a Venomous Effusion wall
    // and every "zg escape venomous effusion" tick returned here while 9 cloud ticks landed (hp 100 -> 9). A bot standing inside an
    // effusion cloud re-plans at once instead of waiting out the old move (FORCED: MoveTo's own IsWaitingForLastMove lets a higher
    // priority through, MovementActions.cpp:329).
    bool const replan = _kind == Zg::HazardKind::VenomousEffusion &&
                        Zg::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) < 0.0f;
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT) && !replan)
        return false;

    if (Zg::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) >= Zg::STAND_MARGIN)
        return false;

    // #670 w68: the tank in a Poison Cloud walks to the Venoxis tank spot instead (it is clear of every cloud); a ring step from the
    // altar finds no spot within 3 yd of its height anyway.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (_kind == Zg::HazardKind::PoisonCloud && Zg::GetVenoxisTankSpot(bot, x, y, z))
        return false;

    // #670 w68: a Venoxis escape (pools, Bloodvenom, Breath) must not end in a Poison Cloud - the mage blinked from the stage to the
    // stair foot and died there in 6 s. Fall back to the plain escape when no spot clears both.
    // #679: nor in a Venomous Effusion wall (and an effusion escape not in a cloud or a pool).
    // #679 w106: a Poison Cloud escape not into a wall either (the rogue stepped off the stairs into one: 2 hits), and every Venoxis
    // escape first looks for a spot whose straight walk crosses no wall (PathClear), then settles for the old end-point-only search.
    std::vector<Zg::Hazard> avoid = hazards;
    bool const venoxisEscape = _kind == Zg::HazardKind::AcridPools || _kind == Zg::HazardKind::Bloodvenom ||
                               _kind == Zg::HazardKind::BreathOfHethiss || _kind == Zg::HazardKind::VenomousEffusion ||
                               _kind == Zg::HazardKind::PoisonCloud;
    if (venoxisEscape)
    {
        if (_kind != Zg::HazardKind::PoisonCloud)
            Zg::CollectHazards(bot, Zg::HazardKind::PoisonCloud, avoid);
        if (_kind != Zg::HazardKind::VenomousEffusion)
            Zg::CollectHazards(bot, Zg::HazardKind::VenomousEffusion, avoid);
        else
            Zg::CollectHazards(bot, Zg::HazardKind::AcridPools, avoid);
    }

    if (!(venoxisEscape && Zg::FindEscapeSpot(bot, avoid, Zg::CLEAR_MARGIN, x, y, z, true)) &&
        !Zg::FindEscapeSpot(bot, avoid, Zg::CLEAR_MARGIN, x, y, z) &&
        (avoid.size() == hazards.size() || !Zg::FindEscapeSpot(bot, hazards, Zg::CLEAR_MARGIN, x, y, z)))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false,
                  replan ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZgBreakToxicLinkAction::Execute(Event /*event*/)
{
    Player* partner = Zg::GetToxicPartner(bot);
    if (!partner)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // The link holds until the two are TOXIC_LINK_BREAK_DISTANCE apart. A tank never runs (GetToxicPartner), so the partner covers
    // the whole remaining distance, at most 10 yd per step.
    float const need = Zg::TOXIC_LINK_BREAK_DISTANCE + 1.0f - bot->GetDistance2d(partner);
    float const step = std::min(std::max(need, 2.0f), 10.0f);

    std::vector<Zg::Hazard> clouds;
    Zg::CollectHazards(bot, Zg::HazardKind::PoisonCloud, clouds);
    Zg::CollectHazards(bot, Zg::HazardKind::VenomousEffusion, clouds); // #679: nor through a maze wall
    if (clouds.empty())
        return MoveAway(partner, step);

    // #670 w68: MoveAway (straight away from the partner, then +-90 deg) sent the healer from the stage up the stairs onto Venoxis's
    // altar, where it died in the Poison Clouds. Same bearings as MoveAway, widened to +-180 deg, minus every step that ends in a
    // cloud or does not widen the gap to the partner.
    float const away = partner->GetAngle(bot);
    float const gap = bot->GetExactDist2d(partner);
    float const eighth = static_cast<float>(M_PI) / 8.0f;
    for (int i = 0; i <= 8; ++i)
    {
        for (float const sign : { 1.0f, -1.0f })
        {
            if (i == 0 && sign < 0.0f)
                continue;

            float const angle = away + sign * eighth * static_cast<float>(i);
            float x = bot->GetPositionX() + std::cos(angle) * step;
            float y = bot->GetPositionY() + std::sin(angle) * step;
            float z = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                                bot->GetPositionZ(), x, y, z))
                continue;

            if (Zg::MinSlack(clouds, x, y) < Zg::CLEAR_MARGIN || partner->GetExactDist2d(x, y) <= gap)
                continue;

            if (MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT, false, false))
                return true;
        }
    }

    return false;
}

bool CataZgVenoxisTankSpotAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!Zg::GetVenoxisTankSpot(bot, x, y, z))
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZgEnterDeadzoneAction::Execute(Event /*event*/)
{
    float centreX = 0.0f;
    float centreY = 0.0f;
    float radius = 0.0f;
    if (!Zg::GetDeadzoneToEnter(bot, centreX, centreY, radius))
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // Half way into the Deadzone, on the line from its centre to the bot.
    float const dist = bot->GetExactDist2d(centreX, centreY);
    float x = centreX;
    float y = centreY;
    if (dist > 0.1f)
    {
        float const reach = radius * 0.5f;
        x = centreX + (bot->GetPositionX() - centreX) / dist * reach;
        y = centreY + (bot->GetPositionY() - centreY) / dist * reach;
    }

    float const bz = bot->GetPositionZ();
    float z = bot->GetMapHeight(x, y, bz + 5.0f, true, 12.0f);
    if (!(std::fabs(z - bz) <= 8.0f))
        z = bz;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZgFocusAction::Execute(Event /*event*/)
{
    Unit* target = Zg::GetFocusTarget(bot);
    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool CataZgPullSpiritAction::Execute(Event /*event*/)
{
    Creature* spirit = Zg::GetSpiritToPull(bot);
    if (!spirit)
        return false;

    // The spirits stand on the four corner terraces below the altar (DB spawns, z 40.8); walk until one is close and in sight.
    if (bot->GetDistance(spirit) <= 25.0f && bot->IsWithinLOSInMap(spirit))
        return Attack(spirit);

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    return MoveTo(spirit->GetMapId(), spirit->GetPositionX(), spirit->GetPositionY(), spirit->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZgStandAtChainAction::Execute(Event /*event*/)
{
    Creature* chain = Zg::GetChainToStandAt(bot);
    if (!chain)
        return false;

    // #670 w65: only the Spirit Warrior's Gaze target stands on the chain; the rest wait on their own spot (CataZgShared.h).
    bool const gazed = Zg::IsGazeTarget(bot);
    float const dist = bot->GetExactDist2d(chain);
    float x = 0.0f;
    float y = 0.0f;
    if (gazed)
    {
        if (dist <= Zg::CHAIN_STAND_RANGE)
            return false;

        // 1.5 yd from the chain on the bot's side, well inside the 5 yd the Body Slam needs (boss_jindo_the_godbreaker.cpp:371).
        float dirX = std::cos(bot->GetOrientation());
        float dirY = std::sin(bot->GetOrientation());
        if (dist > 0.5f)
        {
            dirX = (bot->GetPositionX() - chain->GetPositionX()) / dist;
            dirY = (bot->GetPositionY() - chain->GetPositionY()) / dist;
        }

        x = chain->GetPositionX() + dirX * 1.5f;
        y = chain->GetPositionY() + dirY * 1.5f;
    }
    else
    {
        Zg::GetChainWaitSpot(bot, chain, x, y);
        if (bot->GetExactDist2d(x, y) <= Zg::CHAIN_WAIT_SLACK)
            return false;
    }

    // The Gaze target has ~3 s before the slam lands on it: a forced move is not held back by an earlier combat move.
    MovementPriority const priority = gazed ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float const cz = chain->GetPositionZ();
    float z = bot->GetMapHeight(x, y, cz + 5.0f, true, 12.0f);
    if (!(std::fabs(z - cz) <= 5.0f))
        z = cz;

    if (gazed)
        TC_LOG_DEBUG("playerbots", "[ZGCHAIN] %s gaze run chain=%u dist=%.1f", bot->GetName().c_str(),
                     chain->GetGUID().GetCounter(), dist);

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, priority, true, false);
}

bool CataZgDropSpiritAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!Zg::ShouldSpareSpirit(bot, target))
        return false;

    // DropTargetAction (ChooseTargetActions.cpp:76-104) without the switch to the non-combat engine: the bot is still fighting.
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());
    bot->AttackStop();
    return true;
}

void CataZgCauldronAction::ZgLog(char const* event, float dist, bool force)
{
    uint32 const now = getMSTime();
    if (!force && _lastLogAt && getMSTimeDiff(_lastLogAt, now) < 2000)
        return;

    _lastLogAt = now ? now : 1;
    TC_LOG_DEBUG("playerbots", "[ZGCAULDRON] %s %s dist=%.1f at=%.1f,%.1f,%.1f", bot->GetName().c_str(), event, dist,
                 bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
}

bool CataZgCauldronAction::Execute(Event /*event*/)
{
    Zg::CauldronPlan plan;
    if (!Zg::GetCauldronPlan(bot, plan))
    {
        _bestDist = 1.0e9f;
        _armedAt = 0;
        return false;
    }

    uint32 const now = getMSTime();

    // Carrying the aura: the next damaging spell must land on the Berserker (on Zanzil or Jin'do the charge is wasted,
    // Unit.cpp:8211), so make it the target. The focus action does the same for the other DPS.
    if (plan.armed)
    {
        _bestDist = 1.0e9f;
        float const berserkerDist = bot->GetExactDist(plan.berserker);
        if (!_armedAt)
        {
            _armedAt = now ? now : 1;
            ZgLog("armed", berserkerDist, true);
        }

        // w64: 15 drinks and not one 96331 on the Berserker - the priest's Purify stripped the (Magic) buff, and the proc never
        // landed. When the bot's own spells have not procced it within CAULDRON_PROC_WAIT_MS and the Berserker is in reach, apply
        // what the aura does (the triggered 96331 on the victim, Unit::HandleProcTriggerSpell) and spend the charge.
        if (Zg::CAULDRON_DIRECT_CAST && getMSTimeDiff(_armedAt, now) >= Zg::CAULDRON_PROC_WAIT_MS && berserkerDist <= Zg::CAULDRON_CAST_RANGE &&
            bot->IsWithinLOSInMap(plan.berserker) &&
            (!_lastCastAt || getMSTimeDiff(_lastCastAt, now) >= 1500))
        {
            _lastCastAt = now ? now : 1;
            bot->CastSpell(plan.berserker, Zg::SPELL_FROSTBURN_FORMULA, true);
            bot->RemoveAurasDueToSpell(Zg::SPELL_FROSTBURN_AURA);
            ZgLog(plan.berserker->HasAura(Zg::SPELL_FROSTBURN_FORMULA) ? "cast 96331 stun=1" : "cast 96331 stun=0",
                  berserkerDist, true);
            return true;
        }

        if (AI_VALUE(Unit*, "current target") == plan.berserker)
            return false;

        ZgLog("armed attack berserker", berserkerDist, true);
        return Attack(plan.berserker);
    }

    _armedAt = 0;

    if (_giveUpAt && getMSTimeDiff(_giveUpAt, now) < 60000)
        return false;

    _giveUpAt = 0;
    if (_lastUseAt && getMSTimeDiff(_lastUseAt, now) < 3000)
        return false; // the aura arrives with the use; do not drink twice

    // A fire trail or a fixating Berserker is dealt with by their own actions first.
    std::vector<Zg::Hazard> hazards;
    Zg::CollectHazards(bot, Zg::HazardKind::ZanzilFire, hazards);
    Zg::CollectHazards(bot, Zg::HazardKind::BerserkerPursuit, hazards);
    if (!hazards.empty() && Zg::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) < Zg::CLEAR_MARGIN)
        return false;

    GameObject* cauldron = plan.cauldron;
    float const dist = bot->GetExactDist(cauldron);
    if (dist > INTERACTION_DISTANCE - 0.5f)
    {
        if (dist < _bestDist - 1.0f)
        {
            _bestDist = dist;
            _bestAt = now ? now : 1;
        }

        bool const stalled = getMSTimeDiff(_bestAt, now) > 8000;
        if (!stalled)
        {
            if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
                return true;

            if (MoveTo(cauldron, 2.0f, MovementPriority::MOVEMENT_FORCED))
            {
                ZgLog("walk", dist, false);
                return true;
            }
        }

        // No progress for 8 s or the path failed: the server has no range check on GameObject::Use (the client would), so drink
        // from here when the cauldron is close, else leave it for a minute.
        if (dist > 15.0f)
        {
            ZgLog("giveup", dist, true);
            _giveUpAt = now ? now : 1;
            _bestDist = 1.0e9f;
            return false;
        }

        ZgLog("use-far", dist, true);
    }

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    cauldron->Use(bot);
    _lastUseAt = now ? now : 1;
    _bestDist = 1.0e9f;
    ZgLog(bot->HasAura(Zg::SPELL_FROSTBURN_AURA) ? "drink aura=1" : "drink aura=0", dist, true);
    return true;
}

bool CataZgToxicTormentAction::Execute(Event /*event*/)
{
    GameObject* cauldron = Zg::GetToxicTormentCauldron(bot);
    if (!cauldron)
    {
        _bestDist = 1.0e9f;
        return false;
    }

    uint32 const now = getMSTime();
    if (_lastUseAt && getMSTimeDiff(_lastUseAt, now) < 2000)
        return false; // the aura arrives with the use; do not drink twice

    // A Zanzili Fire trail is dealt with by its own action first (it hits harder than one gas tick).
    std::vector<Zg::Hazard> fire;
    Zg::CollectHazards(bot, Zg::HazardKind::ZanzilFire, fire);
    if (!fire.empty() && Zg::MinSlack(fire, bot->GetPositionX(), bot->GetPositionY()) < Zg::CLEAR_MARGIN)
        return false;

    float const dist = bot->GetExactDist(cauldron);
    if (dist > INTERACTION_DISTANCE - 0.5f)
    {
        if (dist < _bestDist - 1.0f)
        {
            _bestDist = dist;
            _bestAt = now ? now : 1;
        }

        // Same fallback as the Frostburn cauldron (CataZgCauldronAction): no progress for 6 s and close enough -> drink from here.
        bool const stalled = getMSTimeDiff(_bestAt, now) > 6000;
        if (!stalled || dist > 15.0f)
        {
            if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
                return true;

            if (MoveTo(cauldron, 2.0f, MovementPriority::MOVEMENT_FORCED))
            {
                TC_LOG_DEBUG("playerbots", "[ZGTORMENT] %s walk dist=%.1f", bot->GetName().c_str(), dist);
                return true;
            }

            if (dist > 15.0f)
                return false;
        }
    }

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    cauldron->Use(bot);
    _lastUseAt = now ? now : 1;
    _bestDist = 1.0e9f;
    TC_LOG_DEBUG("playerbots", "[ZGTORMENT] %s drink dist=%.1f aura=%u", bot->GetName().c_str(), dist,
                 uint32(bot->HasAura(Zg::SPELL_TOXIC_TORMENT)));
    return true;
}

bool CataZgInterruptAction::isUseful()
{
    // #679 w106: pushed at ACTION_EMERGENCY + 8 (CataZgStrategy.cpp), so it stands aside unless one of the bot's interrupts can land.
    Unit* target = Zg::GetInterruptTarget(bot);
    return target && Zg::CanCastDedicatedInterrupt(botAI, target);
}

bool CataZgInterruptAction::Execute(Event /*event*/)
{
    Unit* target = Zg::GetInterruptTarget(bot);
    if (!target)
        return false;

    return Zg::CastDedicatedInterrupt(botAI, target);
}
