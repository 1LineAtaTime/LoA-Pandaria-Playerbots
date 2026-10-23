/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656). See MopSntShared.h for sources.

#include "MopSntActions.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "LastMovementValue.h"
#include "Log.h"
#include "Map.h"
#include "MopDungeonShared.h"
#include "MotionMaster.h"
#include "ObjectDefines.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <iterator>

namespace
{
char const* EscapeKindName(MopSnt::HazardKind kind)
{
    static char const* const names[] = {"none", "blades", "dash", "bladerush", "bomb", "pitch", "puddle", "leash"};
    uint32 const i = uint32(kind);
    return i < std::size(names) ? names[i] : "?";
}

// #656 w50: the escape outranks every other move (AC Ai/Dungeon/Mech/MechActions.cpp:461-464 and Seth/SethActions.cpp:
// 174-176 leave a hazard with MOVEMENT_FORCED). With MOVEMENT_COMBAT, any combat walk still running (a siege-bomb walk,
// reach melee, the bridge approach) made IsWaitingForLastMove refuse the escape for up to that walk's whole delay
// (MovementActions.cpp:293-305, 398-401): w50 pavalak 310 of 647 escape FAILED came after a siege-bomb walk, 37 after
// reach melee. The healer leash stays MOVEMENT_COMBAT (AC MechActions.cpp:461-462: healers keep COMBAT).
MovementPriority EscapePriority(MopSnt::HazardKind kind)
{
    return kind == MopSnt::HazardKind::HealerLeash ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;
}
}  // namespace

bool MopSntEscapeAction::EscapeInFlight(MopSnt::Hazard const& hazard)
{
    LastMovement const& last = AI_VALUE(LastMovement&, "last movement");
    MovementPriority const priority = EscapePriority(hazard.kind);
    return last.priority == priority && bot->isMoving() && IsWaitingForLastMove(priority) &&
           last.lastMoveToMapId == bot->GetMapId() &&
           std::hypot(last.lastMoveToX - hazard.x, last.lastMoveToY - hazard.y) <= MopSnt::SNT_ESCAPE_SAME_SPOT;
}

bool MopSntEscapeAction::isUseful()
{
    // #656 w50: 260 of the 647 pavalak and 116 of the 231 neronok escape FAILED were the bot still walking to the point
    // its previous escape gave it (<= 6 AI ticks after that OK, [PROBE] moving=1 mgen=8); that walk is not a failure.
    MopSnt::Hazard const hazard = AI_VALUE(MopSnt::Hazard, "mop snt hazard");
    return hazard.kind != MopSnt::HazardKind::None && !EscapeInFlight(hazard);
}

bool MopSntEscapeAction::Execute(Event /*event*/)
{
    MopSnt::Hazard const hazard = AI_VALUE(MopSnt::Hazard, "mop snt hazard");
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    MovementPriority const priority = EscapePriority(hazard.kind);

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();
    float const dist = std::hypot(hazard.x - bx, hazard.y - by);

    // #656 w50 reason-coded line (one per execution): the w50 rows could not tell a refused move from a walk in progress.
    // lastpri / lastleft = the movement the bot had before this call (priority, ms left of its wait).
    uint32 const lastPri = uint32(last.priority);
    int32 const lastLeft = int32(last.lastdelayTime) - int32(getMSTimeDiff(last.msTime, getMSTime()));
    auto const report = [&](char const* result, uint32 tried, bool moved) {
        TC_LOG_DEBUG("playerbots", "[SNTESC] %s kind=%s res=%s d=%.1f lastpri=%u lastleft=%d moving=%u cast=%u tried=%u",
                     bot->GetName().c_str(), EscapeKindName(hazard.kind), result, dist, lastPri, lastLeft,
                     uint32(bot->isMoving()), MopDungeon::CurrentCastSpellId(bot), tried);
        return moved;
    };

    if (hazard.kind == MopSnt::HazardKind::None)
        return report("none", 0, false);

    if (dist < 0.5f)
        return report("at-spot", 0, false);

    // a resin hop in the air (MoveJump): it lands within ~0.3 s, the next tick walks on
    if (bot->IsFalling() || bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
        return report("airborne", 0, false);

    if (!IsMovingAllowed())
        return report("cannot-move", 0, false);

    bool retarget = false;
    if (IsWaitingForLastMove(priority))
    {
        // the leash does not cut another COMBAT walk short
        if (priority != MovementPriority::MOVEMENT_FORCED)
            return report("waiting", 0, false);

        // our own earlier escape to a spot the hazard no longer names (isUseful keeps a walk to the same spot), a landed
        // resin hop or an interrupt chase: MoveTo would refuse the new point until that wait ran out
        last.clear();
        retarget = true;
    }

    // AC SethActions.cpp:174/205: stop the cast before the forced move (a channel would otherwise hold the bot in place)
    if (priority == MovementPriority::MOVEMENT_FORCED && bot->IsNonMeleeSpellCasted(true))
        bot->CastStop();

    // The computed point first, then shorter steps and slightly turned ones: a wall on the straight line must not
    // leave the bot standing in the hazard. Collision check as in UB/UBActions.cpp:35. #781: wider fan (+-1.2 rad)
    // and a longer step - the wave 33 audit showed ~1 in 5 escapes failing with every candidate blocked.
    float const baseAngle = std::atan2(hazard.y - by, hazard.x - bx);
    static float const turns[] = {0.0f, 0.6f, -0.6f, 1.2f, -1.2f};
    static float const scales[] = {1.0f, 0.7f, 1.4f, 0.45f};
    uint32 tried = 0;
    uint32 blocked = 0;
    for (float const turn : turns)
    {
        for (float const scale : scales)
        {
            ++tried;
            float const angle = baseAngle + turn;
            float dx = bx + std::cos(angle) * dist * scale;
            float dy = by + std::sin(angle) * dist * scale;
            float dz = bz;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, dx, dy, dz))
            {
                ++blocked;
                continue;
            }

            if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true, priority))
                return report(retarget ? "moved-retarget" : "moved", tried, true);
        }
    }

    // #656 w53: every exact-waypoint candidate refused. w53 neronok: the healer's leash logged res=moveto-refused tried=20
    // 239 times (out of 279 escape FAILED) - stuck at 1834.1,5218.7 on the deck, 43 yd from the tank who died at the west
    // end without heals. The exact branch refuses any point whose straight path is not "walkable"
    // (MovementActions.cpp PathIsWalkable, #747), which the 20-50 yd leash points along the bridge kept failing; the
    // walk itself is fine. Let the normal path search (SearchForBestPath) route to the hazard's own point instead, after
    // dropping a stalled move so the duplicate-move check does not refuse it as well.
    // #656 w54: that path search to the computed point put the healer ON a bridge structure (moved-path from 1863.4,5205.0
    // z 131.2 to 1866.6,5205.0 z 138.4) where no path led off it: 202 moveto-refused, the tank died alone at the west end.
    // The point itself is the problem, so walk to the tank instead: MoveNear / MoveTo the leash anchor keep the target's
    // (the deck's) z. Only the leash falls back; the other kinds keep their refusal (w54: no pitch refusal at all).
    if (hazard.kind == MopSnt::HazardKind::HealerLeash)
    {
        if (!bot->isMoving())
            last.clear();

        Creature* neronok = bot->FindNearestCreature(MopSnt::SNT_NPC_NERONOK, MopSnt::SNT_NERONOK_CHASE_RANGE);
        Unit* tank = neronok ? neronok->GetVictim() : nullptr;
        if (tank && tank->IsAlive())
        {
            if (MoveNear(tank, MopSnt::SNT_HEALER_LEASH_DIST, priority))
                return report("moved-near", tried + 1, true);

            if (MoveTo(tank, MopSnt::SNT_HEALER_LEASH_DIST, priority))
                return report("moved-path", tried + 2, true);
        }
    }

    return report(blocked == tried ? "no-candidate" : "moveto-refused", tried + 1, false);
}

bool MopSntInterruptAction::isUseful()
{
    // #656 w50: already running to him on the chase move (the trigger only sends a bot that can arrive in time)
    Unit* target = MopSnt::FindInterruptTarget(bot, _group, MopSnt::SNT_NERONOK_CHASE_RANGE);
    if (!target)
        return false;

    if (MopDungeon::CanInterruptNow(bot, target))
        return true;

    LastMovement const& last = AI_VALUE(LastMovement&, "last movement");
    float const range = MopDungeon::ReadyInterruptRange(bot);
    bool const chasing = last.priority == MovementPriority::MOVEMENT_FORCED && bot->isMoving() &&
                         IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED) &&
                         target->GetExactDist2d(last.lastMoveToX, last.lastMoveToY) <= range;
    return !chasing;
}

bool MopSntInterruptAction::Execute(Event /*event*/)
{
    Unit* target = MopSnt::FindInterruptTarget(bot, _group, MopSnt::SNT_NERONOK_CHASE_RANGE);
    float const range = MopDungeon::ReadyInterruptRange(bot);
    // #656 w50 reason-coded line: w50 logged 36 FAILED / 4 OK with nothing to tell why
    auto const report = [&](char const* result, bool done) {
        TC_LOG_DEBUG("playerbots", "[SNTINT] %s res=%s d=%.1f range=%.1f left=%u moving=%u cast=%u",
                     bot->GetName().c_str(), result, target ? bot->GetExactDist(target) : -1.0f, range,
                     target ? MopSnt::CastTimeLeftMs(target, MopSnt::SNT_SPELL_GUSTING_WINDS) : 0u,
                     uint32(bot->isMoving()), MopDungeon::CurrentCastSpellId(bot));
        return done;
    };

    if (!target)
        return report("no-cast", false);

    if (MopDungeon::CanInterruptNow(bot, target))
    {
        // a refusal also logs one [CAST] line per tried spell (PlayerbotAI::CanCastSpell): cooldown / checkcast / range
        if (MopSnt::CastInterrupt(botAI, target))
            return report("cast", true);

        return report("cast-refused", false);
    }

    // #781: out of range - run to him (Wowpedia "Wing Leader Ner'onok": the first to reach him interrupts)
    if (range <= 0.0f)
        return report("not-ready", false);

    if (!MopSnt::CanReachBeforeCastEnds(bot, target, MopSnt::SNT_SPELL_GUSTING_WINDS, range))
        return report("too-far", false);

    // MOVEMENT_FORCED: the bridge approach (MOVEMENT_COMBAT) walks the same way and would refuse a COMBAT chase
    if (MoveNear(target, std::max(range - 1.5f, 1.0f), MovementPriority::MOVEMENT_FORCED))
        return report("chase", true);

    // #656 w48: MoveNear wants LOS from the bot to a point beside him (MovementActions.cpp:590-608); from the near end
    // of the bridge it logged "All paths not in LOS" 73x and the chase FAILED. Walk the path instead, as the approach does.
    if (MoveTo(target, std::max(range - 1.5f, 1.0f), MovementPriority::MOVEMENT_FORCED))
        return report("chase-path", true);

    return report("chase-refused", false);
}

bool MopSntFocusAction::Execute(Event /*event*/)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "mop snt focus target"));
    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool MopSntSiegeBombAction::Execute(Event /*event*/)
{
    Creature* pavalak = MopSnt::GetBulwarkedPavalak(bot);
    if (!pavalak)
        return false;

    // carrying a bomb ("Throw the bomb!", 10 s): throw it at Pa'valak
    if (Aura const* carry = bot->GetAura(MopSnt::SNT_SPELL_SIEGE_EXPLOSIVE_CARRY))
    {
        // #781: one throw per pick-up (the carry aura re-applies the 119386 override every second for 10 s)
        if (carry == _usedCarry && carry->GetApplyTime() == _usedCarryApplied)
            return false;

        if (_lastThrow && GetMSTimeDiffToNow(_lastThrow) < 1500)
            return false;

        if (bot->GetExactDist(pavalak) > 90.0f || !bot->IsWithinLOSInMap(pavalak))
            return false;

        // #781: 119393 is ground-targeted (TRIGGER_MISSILE at TARGET_DEST_DEST, 5.4.8 SpellEffect.dbc), so it is cast at
        // Pa'valak's position; the old unit-targeted cast gave the missile no destination.
        bot->CastSpell(pavalak->GetPositionX(), pavalak->GetPositionY(), pavalak->GetPositionZ(),
                       MopSnt::SNT_SPELL_SIEGE_EXPLOSIVE_THROW, false);
        _lastThrow = getMSTime();

        // the launch removes the 119386 override (spell_siege_explosive_detonate): the throw went out
        if (!bot->HasAura(MopSnt::SNT_SPELL_SIEGE_EXPLOSIVE_OVERRIDE))
        {
            _usedCarry = carry;
            _usedCarryApplied = carry->GetApplyTime();
        }

        return true;
    }

    Creature* bomb = MopSnt::FindClickableBomb(bot);
    if (!bomb)
        return false;

    // #818: surface-to-surface range, as the tar keg below - with GetExactDist <= 4 the walk (MoveTo(bomb, 2.0f) stops at
    // GetDistance 2 = exact 2 + 1.5 + 1.2..2.0 for the 61452 models) never got close enough to click
    if (!bot->IsWithinDistInMap(bomb, INTERACTION_DISTANCE - 1.0f))
        return MoveTo(bomb, 2.0f, MovementPriority::MOVEMENT_COMBAT);

    // 5.4.8: Unit::HandleSpellClick(Unit* clicker, int8 seatId = -1), the same call TOC/TOCActions.cpp:257 makes.
    return bomb->HandleSpellClick(bot);
}

bool MopSntTarKegAction::Execute(Event /*event*/)
{
    if (bot->HasAura(MopSnt::SNT_SPELL_CARRYING_TAR))
    {
        if (_lastThrow && GetMSTimeDiffToNow(_lastThrow) < 1500)
            return false;

        Unit* target = MopSnt::FindTarTarget(bot);
        if (!target)
            return false;

        // ground-targeted like the Siege Explosive throw (TRIGGER_MISSILE at TARGET_DEST_DEST)
        bot->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), MopSnt::SNT_SPELL_TAR_THROW,
                       false);
        _lastThrow = getMSTime();
        TC_LOG_DEBUG("playerbots", "[SNTTAR] %s throw at %u d=%.1f", bot->GetName().c_str(), target->GetEntry(),
                     bot->GetExactDist(target));
        return true;
    }

    Creature* keg = MopSnt::FindClickableKeg(bot);
    if (!keg)
        return false;

    // #656 w55: MOVEMENT_FORCED - with COMBAT the walk was cut by the bots' own reach / dps-assist moves every time: 46 OK
    // (walk started) / 101 FAILED, 0 "[SNTTAR] click keg", 0 casts of 123039 in the Vo'jak leg
    // #818: the range test is surface to surface, the same measure MoveTo(keg, 2.0f) walks by (WorldObject::GetDistance =
    // exact distance - both combat reaches, Object.cpp:1643). The old centre-to-centre GetExactDist <= 4 could never be met:
    // the walk stops at GetDistance 2 = exact 2 + 1.5 (player) + 1.5 (keg model 42665 combat_reach) = 5 yd, the next MoveTo
    // has nothing left to walk, so w60 logged 67 OK / 96 FAILED and 0 clicks while the trigger was active 164 ticks.
    if (!bot->IsWithinDistInMap(keg, INTERACTION_DISTANCE - 1.0f))
        return MoveTo(keg, 2.0f, MovementPriority::MOVEMENT_FORCED);

    // 5.4.8: Unit::HandleSpellClick(Unit* clicker, int8 seatId = -1), as the Siege Explosive pick-up above. The click casts
    // 123032 on the bot (npc_spellclick_spells cast_flags 3) and npc_mantid_tar_keg::OnSpellClick despawns the keg.
    bool const clicked = keg->HandleSpellClick(bot);
    TC_LOG_DEBUG("playerbots", "[SNTTAR] %s click keg d=%.1f result=%u carry=%u", bot->GetName().c_str(),
                 bot->GetExactDist(keg), clicked ? 1u : 0u, bot->HasAura(MopSnt::SNT_SPELL_CARRYING_TAR) ? 1u : 0u);
    return clicked;
}

bool MopSntSoakPuddleAction::Execute(Event /*event*/)
{
    Creature* puddle = nullptr;
    if (!MopSnt::WantsToSoakPuddle(bot, puddle))
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // a point in the puddle's inner half, on the bot's side of it
    float const radius = MopSnt::SapPuddleRadius(puddle);
    float const angle = puddle->GetAngle(bot);
    float const x = puddle->GetPositionX() + std::cos(angle) * radius * 0.4f;
    float const y = puddle->GetPositionY() + std::sin(angle) * radius * 0.4f;
    float z = bot->GetMapWaterOrGroundLevel(x, y, puddle->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = puddle->GetPositionZ();

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT);
}

bool MopSntNeronokApproachAction::Execute(Event /*event*/)
{
    Creature* neronok = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    bool flying = false;
    if (!MopSnt::NeronokRegroupPoint(bot, neronok, x, y, flying) || IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // MovementAction::MoveTo(WorldObject*, distance) walks at most spellDistance per call; the trigger repeats it
    // #656 w55: the tank walks into melee of the landed Ner'onok (the stop NeronokRegroupPoint uses), the others to the approach
    // distance; MoveTo(WorldObject*) walks at most spellDistance per call and the trigger repeats it
    if (!flying)
        return MoveTo(neronok,
                      PlayerBotSpec::IsTank(bot) ? MopSnt::SNT_NERONOK_TANK_LANDING_DIST : MopSnt::SNT_NERONOK_APPROACH_DIST,
                      MovementPriority::MOVEMENT_COMBAT);

    // #656 w50: he is still in the air - walk to the end he lands at (Wowpedia: "Chase Ner'onok once he lifts off").
    // w50: the tank only set off once he had landed and crossed ~110 yd under Hurl Brick (121762, every 2 s while his
    // victim is out of melee, boss_wing_leader_neronok.cpp:213-218): 69% -> 19% on the first crossing, 82% -> 13% on
    // the second, dead 6 s after arriving. The tank stops in melee reach of the landing point, the others at the approach
    // distance on their side of it.
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const d = std::hypot(x - bx, y - by);
    float const stop = PlayerBotSpec::IsTank(bot) ? MopSnt::SNT_NERONOK_TANK_LANDING_DIST : MopSnt::SNT_NERONOK_APPROACH_DIST;
    if (d <= stop + 1.0f)
        return false;

    float const tx = bx + (x - bx) / d * (d - stop);
    float const ty = by + (y - by) / d * (d - stop);
    return MoveTo(bot->GetMapId(), tx, ty, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool MopSntResinHopAction::Execute(Event /*event*/)
{
    // in the air already (a fall or our own MoveJump arc): one take-off per hop
    if (bot->IsFalling() || bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
        return false;

    if (_lastHop && GetMSTimeDiffToNow(_lastHop) < MopSnt::SNT_RESIN_HOP_INTERVAL)
        return false;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();

    // direction: out of a hazard when there is one, else ahead / back alternately (a repeated target would also be
    // dropped as a duplicate move by JumpTo)
    float angle = bot->GetOrientation();
    MopSnt::Hazard const hazard = AI_VALUE(MopSnt::Hazard, "mop snt hazard");
    if (hazard.kind != MopSnt::HazardKind::None && std::hypot(hazard.x - bx, hazard.y - by) > 0.5f)
        angle = std::atan2(hazard.y - by, hazard.x - bx);
    else if (_back)
        angle += float(M_PI);

    // #656 w50: the escape and the interrupt chase now walk with MOVEMENT_FORCED too; their wait must not hold the hop back
    // (JumpTo refuses while IsWaitingForLastMove(FORCED), MovementActions.cpp JumpTo). The hop's own 1000 ms wait is over
    // by now (SNT_RESIN_HOP_INTERVAL above), so only such a walk is cut - and the hop heads for the escape point anyway.
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
        AI_VALUE(LastMovement&, "last movement").clear();

    for (float const turn : {0.0f, float(M_PI)})
    {
        float dx = bx + std::cos(angle + turn) * MopSnt::SNT_RESIN_HOP_DIST;
        float dy = by + std::sin(angle + turn) * MopSnt::SNT_RESIN_HOP_DIST;
        float dz = bz;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, dx, dy, dz))
            continue;

        // FORCED: the hop is the whole answer to the resin, no walking order may swallow it
        if (JumpTo(bot->GetMapId(), dx, dy, dz, MovementPriority::MOVEMENT_FORCED))
        {
            _lastHop = getMSTime();
            _back = !_back;
            return true;
        }
    }

    return false;
}
