/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - hazard / focus / interrupt computations.
// See MopSntShared.h for the sources of every mechanic and id.

#include "MopSntShared.h"

#include "AttackersValue.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <list>
#include <string>
#include <vector>

namespace MopSnt
{
namespace
{
// Nearest living creature of `entry` within `range` (centre-to-centre) that satisfies `pred`.
template <class Pred>
Creature* NearestCreature(Player* bot, uint32 entry, float range, Pred pred)
{
    std::list<Creature*> found;
    // 5.4.8: WorldObject::GetCreatureListWithEntryInGrid(list&, entry, range), same call as UB/UBShared.cpp.
    bot->GetCreatureListWithEntryInGrid(found, entry, range);

    Creature* best = nullptr;
    float bestDist = range + 1.0f;
    for (Creature* creature : found)
    {
        if (!creature || !creature->IsAlive() || !pred(creature))
            continue;

        float const dist = bot->GetExactDist2d(creature);
        if (dist < bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

// The Spell* of `spellId` while `unit` is casting or channeling it, else nullptr. Same lookup as
// HoS/HoSTriggers.cpp:23 (FindCurrentSpellBySpellId); the spell state keeps a finished / in-flight spell out.
Spell* CurrentCast(Unit* unit, uint32 spellId)
{
    if (!unit)
        return nullptr;

    Spell* spell = unit->FindCurrentSpellBySpellId(spellId);
    if (!spell)
        return nullptr;

    uint32 const state = spell->getState();
    if (state != SPELL_STATE_PREPARING && state != SPELL_STATE_CASTING)
        return nullptr;

    return spell;
}

// Largest effect radius of a spell (UB/UBShared.cpp MaxEffectRadius), clamped to a sane band.
float ClampedRadius(uint32 spellId, float fallback, float lo, float hi)
{
    float radius = 0.0f;
    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            radius = std::max(radius, spellInfo->Effects[i].CalcRadius());
    }

    if (radius <= 0.0f)
        radius = fallback;

    return std::clamp(radius, lo, hi);
}

// A point `safeDist` away from (ox, oy) on the line through the bot. False when the bot is already clear.
bool SetAway(Player* bot, float ox, float oy, float safeDist, HazardKind kind, Hazard& out)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const dist = std::hypot(bx - ox, by - oy);
    if (dist >= safeDist)
        return false;

    float ux, uy;
    if (dist < 0.1f)
    {
        ux = std::cos(bot->GetOrientation());
        uy = std::sin(bot->GetOrientation());
    }
    else
    {
        ux = (bx - ox) / dist;
        uy = (by - oy) / dist;
    }

    float const step = safeDist - dist + 1.0f;
    out.kind = kind;
    out.x = bx + ux * step;
    out.y = by + uy * step;
    return true;
}

// Sideways step out of a cone (halfAngle > 0) or a lane (halfAngle 0) that starts at (ox, oy) and points along
// `facing`. `halfWidth` is the lane half width / the cone's width at its tip, `length` the reach (0 = unlimited).
// False when the bot is outside the area.
bool EscapeFromArea(float ox, float oy, float facing, float halfAngle, float halfWidth, float length, float margin,
                    float bx, float by, float& outX, float& outY)
{
    float const dx = bx - ox;
    float const dy = by - oy;
    float const fx = std::cos(facing);
    float const fy = std::sin(facing);
    float const forward = dx * fx + dy * fy;
    float const lateral = -dx * fy + dy * fx;  // positive = left of the facing

    if (forward < -halfWidth)
        return false;

    if (length > 0.0f && forward > length + margin)
        return false;

    float const reach = std::max(forward, 0.0f);
    float const width = halfWidth + reach * std::tan(halfAngle) + margin;
    float const absLateral = std::fabs(lateral);
    if (absLateral >= width)
        return false;

    float const side = lateral >= 0.0f ? 1.0f : -1.0f;
    float const step = width - absLateral + 1.0f;
    outX = bx - fy * side * step;
    outY = by + fx * side * step;
    return true;
}

bool AreaHazard(Player* bot, float ox, float oy, float facing, float halfAngle, float halfWidth, float length,
                float margin, HazardKind kind, Hazard& out)
{
    float ex, ey;
    if (!EscapeFromArea(ox, oy, facing, halfAngle, halfWidth, length, margin, bot->GetPositionX(),
                        bot->GetPositionY(), ex, ey))
        return false;

    out.kind = kind;
    out.x = ex;
    out.y = ey;
    return true;
}

// Vo'jak's Thousand Blades: a 4 s whirlwind on everybody near him. Wowpedia: "Move away from Vo'jak".
bool BladesHazard(Player* bot, Hazard& out)
{
    Creature* vojak = bot->FindNearestCreature(SNT_NPC_VOJAK, 60.0f);
    if (!vojak || !CurrentCast(vojak, SNT_SPELL_THOUSAND_BLADES))
        return false;

    float const safe = ClampedRadius(SNT_SPELL_THOUSAND_BLADES, 8.0f, 6.0f, 11.0f) + 2.0f;
    return SetAway(bot, vojak->GetPositionX(), vojak->GetPositionY(), safe, HazardKind::Blades, out);
}

// Vo'jak's Dashing Strike: a charge at a random player that damages everybody on the line. Wowpedia: "Move out of
// his charging path".
bool DashHazard(Player* bot, Hazard& out)
{
    Creature* vojak = bot->FindNearestCreature(SNT_NPC_VOJAK, 70.0f);
    Spell* dash = CurrentCast(vojak, SNT_SPELL_DASHING_STRIKE);
    if (!dash)
        return false;

    ObjectGuid const targetGuid = dash->m_targets.GetUnitTargetGUID();
    if (targetGuid.IsEmpty() || targetGuid == bot->GetGUID())
        return false;

    Unit* target = ObjectAccessor::GetUnit(*bot, targetGuid);
    if (!target)
        return false;

    float const length = vojak->GetExactDist2d(target) + 3.0f;
    return AreaHazard(bot, vojak->GetPositionX(), vojak->GetPositionY(), vojak->GetAngle(target), 0.0f, 3.5f, length,
                      1.5f, HazardKind::Dash, out);
}

// Pa'valak's Blade Rush: the sword lands on a player's spot (175000 within 4 yd), then he charges to it (277500+
// within 8 yd, knock back). Stay out of the landing disc and out of the charge line.
bool BladeRushHazard(Player* bot, Hazard& out)
{
    Creature* pavalak = bot->FindNearestCreature(SNT_NPC_PAVALAK, 90.0f);
    Creature* sword = bot->FindNearestCreature(SNT_NPC_BLADE_RUSH_STALKER, 90.0f);
    if (!pavalak || !sword)
        return false;

    // the stalker lingers for 15 s, the rush is over after ~6 s
    if (TempSummon* summon = sword->ToTempSummon())
    {
        if (summon->GetTimer() < SNT_BLADE_RUSH_MIN_TIMER)
            return false;
    }

    if (SetAway(bot, sword->GetPositionX(), sword->GetPositionY(), SNT_BLADE_RUSH_DISC, HazardKind::BladeRush, out))
        return true;

    float const length = pavalak->GetExactDist2d(sword) + 2.0f;
    return AreaHazard(bot, pavalak->GetPositionX(), pavalak->GetPositionY(), pavalak->GetAngle(sword), 0.0f,
                      SNT_BLADE_RUSH_LANE, length, 1.5f, HazardKind::BladeRush, out);
}

// An armed Siege Explosive detonates 4 s after it is armed (277500-322500 Fire): get away from it.
bool BombHazard(Player* bot, Hazard& out)
{
    float const safe = ClampedRadius(SNT_SPELL_BOMB_DETONATE, 8.0f, 6.0f, 10.0f) + 1.5f;
    Creature* bomb = NearestCreature(bot, SNT_NPC_SIEGE_EXPLOSIVE, SNT_SCAN,
                                     [](Creature* creature) { return creature->HasAura(SNT_SPELL_BOMB_ARMED); });
    if (!bomb)
        return false;

    return SetAway(bot, bomb->GetPositionX(), bomb->GetPositionY(), safe, HazardKind::Bomb, out);
}

// Ner'onok's Caustic Pitch: a ground pool (dynamic object of spell 121443), 40000 Nature per second and -50% speed.
bool PitchHazard(Player* bot, Hazard& out)
{
    float const safe = ClampedRadius(SNT_SPELL_CAUSTIC_PITCH, 5.0f, 3.5f, 9.0f) + 1.5f;
    std::vector<Position> pools = EncounterHelpers::GetDynamicObjectPositions(bot, SNT_SCAN, SNT_SPELL_CAUSTIC_PITCH);

    Position const* nearest = nullptr;
    float nearestDist = safe;
    for (Position const& pool : pools)
    {
        float const dist = bot->GetExactDist2d(pool);
        if (dist < nearestDist)
        {
            nearest = &pool;
            nearestDist = dist;
        }
    }

    if (!nearest)
        return false;

    // #781 round 2: the tank leaves the pool but stays in Ner'onok's melee range. Wowpedia "Wing Leader Ner'onok"
    // (fetched 2026-10-11): "As a tank make sure to always stay in Wing Leader Ner'onok his melee range. He will toss
    // bricks at you if you don't." - Hurl Brick (121762) every 2 s at a victim that is not IsWithinMeleeRange
    // (boss_wing_leader_neronok.cpp:213-218). Wave 35: the tank's plain escape took him out of melee, the escape
    // FAILED 64 / OK 26 for him and he died in ~50 s.
    //
    // #656 w48: on the bridge the straight "away from the pool" step (+-69 deg fan in the escape) mostly ends off the
    // deck: escape FAILED 319 / OK 43 in the leg, and the hazard multiplier vetoed every other move meanwhile - after
    // the 70% flight the mage stood 16 s in one pool (1894.6,5169.4, run speed x0.57) and died, the tank crawled and
    // died. So every bot samples the full circle for a reachable spot clear of all pools (AC
    // Ai/Base/Actions/MovementActions.cpp:2123-2144 BestPositionForRangedToFlee also tries toward / away from the
    // target, i.e. along the bridge); with none reachable there is no hazard, so the bot is not frozen in the pool.
    Creature* neronok = bot->FindNearestCreature(SNT_NPC_NERONOK, 40.0f);
    bool const holdMelee = PlayerBotSpec::IsTank(bot) && neronok && neronok->IsInCombat() && neronok->GetVictim() == bot;
    // Unit::IsWithinMeleeRange (Unit.cpp:611-626): max(5, both combat reaches + 4/3); 0.75 yd margin
    float const melee = holdMelee ? std::max(NOMINAL_MELEE_RANGE, bot->GetCombatReach() + neronok->GetCombatReach() + 4.0f / 3.0f) - 0.75f : 0.0f;
    float const nx = holdMelee ? neronok->GetPositionX() : 0.0f;
    float const ny = holdMelee ? neronok->GetPositionY() : 0.0f;
    // #656 w52: while the bot crosses the bridge to Ner'onok (NeronokRegroupPoint), no spot farther from where it is going.
    // w52: the tank's approach (MOVEMENT_COMBAT) walked into a pool on the deck, the FORCED escape sent it back out the
    // way it came, the approach walked in again - 8 approach OK / 8 escape OK at 1857-1864,5210-5222 for ~25 s under
    // Hurl Brick (121762), dead 63 yd short of him. With no forward or sideways spot clear of the pools there is no
    // hazard and the approach walks through (MopSntHazardMultiplier exempts it from a Pitch hazard).
    Creature* crossing = nullptr;
    float rx = 0.0f;
    float ry = 0.0f;
    bool airborne = false;
    bool const crossingBridge = NeronokRegroupPoint(bot, crossing, rx, ry, airborne);
    float const toGo = std::hypot(rx - bot->GetPositionX(), ry - bot->GetPositionY());
    auto const score = [&pools, holdMelee, nx, ny, melee, safe, crossingBridge, rx, ry, toGo](float px, float py) {
        if (holdMelee && std::hypot(px - nx, py - ny) > melee)
            return -100.0f;

        if (crossingBridge && std::hypot(px - rx, py - ry) > toGo + 1.0f)
            return -100.0f;

        float clear = safe + 2.0f;  // was 8.0f: below `safe` whenever the pitch radius is over 6.5 yd
        for (Position const& pool : pools)
            clear = std::min(clear, std::hypot(px - pool.GetPositionX(), py - pool.GetPositionY()));

        return clear;
    };

    // #656 w50: keep the spot the escape is already walking to while it is still clear. FindBestSpot samples rings around
    // the MOVING bot at every refresh of this value (250 ms), so the best spot drifts and the escape saw a new point each
    // time instead of finishing its walk (w50 neronok: escape FAILED 231 / OK 62; 116 of the FAILED came <= 6 AI ticks
    // after the bot's own escape move, the bot still walking).
    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
    {
        LastMovement const& last = botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
        if (last.priority == MovementPriority::MOVEMENT_FORCED && bot->isMoving() && last.lastMoveToMapId == bot->GetMapId() &&
            float(getMSTimeDiff(last.msTime, getMSTime())) < last.lastdelayTime && score(last.lastMoveToX, last.lastMoveToY) >= safe)
        {
            out.kind = HazardKind::Pitch;
            out.x = last.lastMoveToX;
            out.y = last.lastMoveToY;
            return true;
        }
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (MopDungeon::FindBestSpot(bot, score, 0.5f, x, y, z) && score(x, y) >= safe)
    {
        out.kind = HazardKind::Pitch;
        out.x = x;
        out.y = y;
        return true;
    }

    if (!holdMelee)
        return false;

    return SetAway(bot, nearest->GetPositionX(), nearest->GetPositionY(), safe, HazardKind::Pitch, out);
}

// #656 w48: a healer more than SNT_HEALER_LEASH_MAX from Ner'onok's victim, or without LOS to it, walks back toward it.
// After the 70% flight the healer walked the bridge ahead of everybody (approach) and stood 43 yd from the tank and
// 93 yd from the mage while both died. Lowest priority of all hazards; same shape as MopMspShared.cpp
// HealerLeashHazard (#654, AC Dungeon/Mech/MechActions.cpp:216-218: healer leashed to the tank).
bool HealerLeashHazard(Player* bot, Hazard& out)
{
    if (!PlayerBotSpec::IsHeal(bot))
        return false;

    Group* group = bot->GetGroup();
    Creature* neronok = bot->FindNearestCreature(SNT_NPC_NERONOK, SNT_NERONOK_CHASE_RANGE);
    if (!group || !neronok || !neronok->IsInCombat())
        return false;

    Unit* tank = neronok->GetVictim();
    if (!tank || tank == bot || !tank->IsAlive() || tank->GetTypeId() != TYPEID_PLAYER || !group->IsMember(tank->GetGUID()))
        return false;

    float const dist = bot->GetExactDist2d(tank);
    if (dist < 1.0f || (dist <= SNT_HEALER_LEASH_MAX && bot->IsWithinLOSInMap(tank)))
        return false;

    float const keep = std::min(SNT_HEALER_LEASH_DIST, dist * 0.5f);
    out.kind = HazardKind::HealerLeash;
    out.x = tank->GetPositionX() + (bot->GetPositionX() - tank->GetPositionX()) / dist * keep;
    out.y = tank->GetPositionY() + (bot->GetPositionY() - tank->GetPositionY()) / dist * keep;
    return true;
}

// Jin'bak's Sap Puddle (#781): a bot carrying SNT_RESIDUE_MAX Sap Residue stacks leaves it.
// #656 w47: the tank never stands in it ("Tank Vizier Jin'bak near the middle of the room", Wowpedia - near, not in:
// the soak is the DPS's job). w47: Jolerie tanked 4.3 yd from the centre (radius 6.4 yd) from 76 s on, Sap Residue
// reached 24 stacks (72000 a tick, 813k in all) and the tank died at 100 s; the wipe followed. He steps out and Jin'bak
// follows his tank - but never past Jin'bak's room leash (boss_vizier_jinbak.cpp:272, 30 yd from the puddle centre).
bool PuddleHazard(Player* bot, Hazard& out)
{
    bool const tank = PlayerBotSpec::IsTank(bot);
    if (!tank && SapResidueStacks(bot) < SNT_RESIDUE_MAX)
        return false;

    Creature* puddle = FindSapPuddle(bot);
    if (!puddle)
        return false;

    float const safe = SapPuddleRadius(puddle) + 1.5f;
    if (tank && safe > SNT_JINBAK_TANK_MAX_DIST)
        return false;

    return SetAway(bot, puddle->GetPositionX(), puddle->GetPositionY(), safe, HazardKind::Puddle, out);
}

struct InterruptRule
{
    uint32 entry;
    uint32 spellId;
};

// Ner'onok's Gusting Winds (4 s channel). He is only interruptible while the channel runs
// (boss_wing_leader_neronok.cpp:92-93, 383-396 lifts his interrupt immunity for the length of the aura).
InterruptRule const NERONOK_RULES[] = {{SNT_NPC_NERONOK, SNT_SPELL_GUSTING_WINDS}};

// MoP interrupt spell names the class strategies register (kick, pummel, wind shear, mind freeze, counterspell,
// spell lock, skull bash, spear hand strike, rebuke, counter shot, silencing shot, silence). #781: "counter shot"
// added - the 5.4 hunter interrupt (147362) replaced Silencing Shot as the baseline one.
std::vector<std::string> const& InterruptSpellNames()
{
    static std::vector<std::string> const names = {"kick",           "pummel",           "wind shear",
                                                   "mind freeze",    "counterspell",     "spell lock",
                                                   "skull bash",     "spear hand strike", "rebuke",
                                                   "counter shot",   "silencing shot",   "silence"};
    return names;
}
}  // namespace

bool ComputeHazard(Player* bot, Hazard& out)
{
    out = Hazard();
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return false;

    return BladesHazard(bot, out) || DashHazard(bot, out) || BladeRushHazard(bot, out) || BombHazard(bot, out) ||
           PitchHazard(bot, out) || PuddleHazard(bot, out) || HealerLeashHazard(bot, out);
}

namespace
{
// #656 w53 Vo'jak waves: a ranged DPS bot shoots the nearest Sik'thik Demolisher on the ramp while no group member is within
// its Unstable Blast (Wowpedia: "shoot them before they reach the top"). Before the in-combat check on purpose: the group
// waits on the platform out of combat while a wave climbs, and the idle wave spawns (1485-1538,5387-5407) are 85+ yd from
// the platform, outside the 40 yd scan.
Creature* DemolisherToShoot(Player* bot)
{
    if (!PlayerBotSpec::IsRanged(bot) || PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return nullptr;

    Group* group = bot->GetGroup();
    return NearestCreature(bot, SNT_NPC_SIKTHIK_DEMOLISHER, 40.0f, [bot, group](Creature* creature) {
        if (!bot->IsWithinLOSInMap(creature) || !AttackersValue::IsValidTarget(creature, bot))
            return false;

        if (!group)
            return bot->GetExactDist2d(creature) > SNT_DEMOLISHER_BLAST + 1.0f;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->IsInMap(creature) &&
                member->GetExactDist2d(creature) <= SNT_DEMOLISHER_BLAST + 1.0f)
                return false;
        }

        return true;
    });
}
}  // namespace

ObjectGuid ComputeFocusTarget(Player* bot)
{
    if (bot && bot->IsAlive())
    {
        if (Creature* demolisher = DemolisherToShoot(bot))
            return demolisher->GetGUID();
    }

    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return ObjectGuid::Empty;

    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return ObjectGuid::Empty;

    // #781 Pa'valak behind his Bulwark: every DPS on him (the shield absorbs all schools). The bomb throw ranks above
    // this focus (ACTION_RAID + 4 vs + 2), so a bot with a bomb in reach uses it first; the tank holds the soldiers.
    if (Creature* pavalak = GetBulwarkedPavalak(bot))
    {
        if (AttackersValue::IsValidTarget(pavalak, bot))
            return pavalak->GetGUID();
    }

    // Only at Jin'bak: his Sap Globules crawl to the Sap Puddle and make the Detonate hit harder.
    if (!bot->FindNearestCreature(SNT_NPC_JINBAK, 70.0f))
        return ObjectGuid::Empty;

    // #656 w47: ranged DPS reach every globule from where they stand (the 61629 spawn points are 32-39 yd from the
    // puddle), so they shoot the one closest to the puddle - the next to merge; melee keep the nearest one (no run
    // across the room). w47 waves 1-2: the third globule was last targeted 16-17 s after its wave spawned, against a
    // 2 yd/s walk of 14-17 s to the puddle edge - no margin left.
    if (PlayerBotSpec::IsRanged(bot))
    {
        if (Creature* puddle = FindSapPuddle(bot))
        {
            std::list<Creature*> globules;
            bot->GetCreatureListWithEntryInGrid(globules, SNT_NPC_SAP_GLOBULE, 60.0f);
            Creature* urgent = nullptr;
            float urgentDist = 0.0f;
            for (Creature* creature : globules)
            {
                if (!creature || !creature->IsAlive() || !AttackersValue::IsValidTarget(creature, bot))
                    continue;

                float const dist = creature->GetExactDist2d(puddle);
                if (!urgent || dist < urgentDist)
                {
                    urgent = creature;
                    urgentDist = dist;
                }
            }

            if (urgent)
                return urgent->GetGUID();
        }
    }

    // #656 w51: melee only take a globule that is already inside the room's safe core. w51 the rogue chased globule
    // 61623:408 to 1518.2,5191.8 (30.7 yd from the puddle centre, ~20 yd from the NW Resin Flake pack 1505-1512,5208-5217):
    // attackers 0 -> 7, the flakes (61910, 99 target ticks) reached the tank and the group wiped at Jin'bak 40% (w50: 0
    // trash ticks, kill). The globules crawl to the puddle, so melee meet them there; the ranged shoot the far ones.
    Creature* puddle = FindSapPuddle(bot);
    Creature* globule = NearestCreature(bot, SNT_NPC_SAP_GLOBULE, 60.0f, [bot, puddle](Creature* creature) {
        if (puddle && creature->GetExactDist2d(puddle) > SNT_GLOBULE_MELEE_LEASH)
            return false;

        return AttackersValue::IsValidTarget(creature, bot);
    });

    return globule ? globule->GetGUID() : ObjectGuid::Empty;
}

Unit* FindInterruptTarget(Player* bot, InterruptGroup /*group*/, float range)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return nullptr;

    for (InterruptRule const& rule : NERONOK_RULES)
    {
        uint32 const spellId = rule.spellId;
        Creature* caster = NearestCreature(bot, rule.entry, range, [spellId](Creature* creature) {
            return CurrentCast(creature, spellId) != nullptr;
        });
        if (caster)
            return caster;
    }

    return nullptr;
}

bool HasInterruptSpell(PlayerbotAI* botAI)
{
    for (std::string const& name : InterruptSpellNames())
    {
        if (botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get())
            return true;
    }

    return false;
}

bool CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    for (std::string const& name : InterruptSpellNames())
    {
        if (!botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get())
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return true;
    }

    return false;
}

Creature* GetBulwarkedPavalak(Player* bot)
{
    Creature* pavalak = bot->FindNearestCreature(SNT_NPC_PAVALAK, 100.0f);
    if (!pavalak || !pavalak->HasAura(SNT_SPELL_BULWARK))
        return nullptr;

    return pavalak;
}

Creature* FindClickableBomb(Player* bot)
{
    // #781 round 2: skip a bomb that lies inside the blast of an armed one (119703 Detonate, 7 yd, SpellRadius 37) -
    // the walk there would end in the hazard the escape just left (wave 35: 279 "hazard made action mop snt siege
    // bomb useless").
    float const blast = ClampedRadius(SNT_SPELL_BOMB_DETONATE, 8.0f, 6.0f, 10.0f) + 1.5f;
    std::list<Creature*> bombs;
    bot->GetCreatureListWithEntryInGrid(bombs, SNT_NPC_SIEGE_EXPLOSIVE, 45.0f + blast);
    std::vector<Position> armed;
    for (Creature* bomb : bombs)
    {
        if (bomb && bomb->IsAlive() && bomb->HasAura(SNT_SPELL_BOMB_ARMED))
            armed.push_back(bomb->GetPosition());
    }

    // #656 w50: only a bomb the bot reaches before it arms. w50 pavalak: 310 of the 647 escape FAILED came after a siege-bomb
    // walk (MoveTo the bomb, MOVEMENT_COMBAT, up to ~5 s): the bomb armed under the walking bot, which kept walking into
    // the blast while that move's wait refused the escape's own MoveTo.
    float const speed = std::max(bot->GetSpeed(MOVE_RUN), 1.0f);
    return NearestCreature(bot, SNT_NPC_SIEGE_EXPLOSIVE, 45.0f, [bot, &armed, blast, speed](Creature* creature) {
        if (creature->HasAura(SNT_SPELL_BOMB_ARMED) || !creature->HasNpcFlag(UNIT_NPC_FLAG_SPELLCLICK) ||
            creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            return false;

        float const walk = std::max(0.0f, bot->GetExactDist2d(creature) - (INTERACTION_DISTANCE - 1.0f));
        if (walk / speed * 1000.0f + float(SNT_BOMB_CLICK_MARGIN_MS) > float(BombClickableLeftMs(creature)))
            return false;

        for (Position const& pos : armed)
        {
            if (creature->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) < blast)
                return false;
        }

        return true;
    });
}

Creature* FindNeronokAcrossBridge(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return nullptr;

    Creature* neronok = bot->FindNearestCreature(SNT_NPC_NERONOK, SNT_NERONOK_CHASE_RANGE);
    if (!neronok || !neronok->IsInCombat() || neronok->HasReactState(REACT_PASSIVE))
        return nullptr;

    if (bot->GetExactDist2d(neronok) <= SNT_NERONOK_REGROUP_DIST)
        return nullptr;

    return neronok;
}

bool NeronokRegroupPoint(Player* bot, Creature*& neronok, float& x, float& y, bool& flying)
{
    neronok = nullptr;
    flying = false;
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return false;

    Creature* found = bot->FindNearestCreature(SNT_NPC_NERONOK, SNT_NERONOK_CHASE_RANGE);
    if (!found || !found->IsInCombat())
        return false;

    neronok = found;
    if (!found->HasReactState(REACT_PASSIVE))
    {
        // landed (EVENT_RE_ENGAGE lifts REACT_PASSIVE): walk to him. #656 w55: no longer only to SNT_NERONOK_REGROUP_DIST - the
        // tank stopped 40-43 yd short (d1 40-43, 79 "neronok far" ticks), the trigger went off there, the bot never got an
        // attacker ("T:no attackers" 253x, target=-), the pitch escape (no crossing constraint inside 40 yd) and the approach
        // ping-ponged (escape OK 51 / approach OK 35), and Hurl Brick (60-81k every 2 s) killed him 40 yd from Ner'onok while
        // the healer burnt 55% -> 3% mana on him. Now every bot walks until it stands where the approach stops.
        x = found->GetPositionX();
        y = found->GetPositionY();
        float const landedStop = PlayerBotSpec::IsTank(bot) ? SNT_NERONOK_TANK_LANDING_DIST + 1.0f : SNT_NERONOK_APPROACH_DIST;
        return bot->GetExactDist2d(found) > landedStop + 1.0f;
    }

    // In the air (REACT_PASSIVE from the take-off, boss_wing_leader_neronok.cpp:126-139, to EVENT_RE_ENGAGE): the end his
    // flight spline heads for; before / after the long leg (the in-place first leg, the vertical MoveLand) the 70% flight
    // (hp above 40%) lands at bridgePos[1], the 40% flight at bridgePos[0].
    flying = true;
    bool toEnd1 = found->GetHealthPct() > 40.0f;
    if (!found->movespline->Finalized())
    {
        auto const dest = found->movespline->FinalDestination();
        if (std::hypot(dest.x - found->GetPositionX(), dest.y - found->GetPositionY()) > 5.0f)
            toEnd1 = std::hypot(dest.x - SNT_BRIDGE_END1_X, dest.y - SNT_BRIDGE_END1_Y) <
                     std::hypot(dest.x - SNT_BRIDGE_END0_X, dest.y - SNT_BRIDGE_END0_Y);
    }

    x = toEnd1 ? SNT_BRIDGE_END1_X : SNT_BRIDGE_END0_X;
    y = toEnd1 ? SNT_BRIDGE_END1_Y : SNT_BRIDGE_END0_Y;
    // until the bot stands where MopSntNeronokApproachAction stops (tank: melee reach of the landing point)
    float const stop = PlayerBotSpec::IsTank(bot) ? SNT_NERONOK_TANK_LANDING_DIST : SNT_NERONOK_APPROACH_DIST;
    return std::hypot(x - bot->GetPositionX(), y - bot->GetPositionY()) > stop + 1.0f;
}

Creature* FindClickableKeg(Player* bot)
{
    return NearestCreature(bot, SNT_NPC_MANTID_TAR_KEG, 40.0f, [](Creature* keg) {
        return keg->HasNpcFlag(UNIT_NPC_FLAG_SPELLCLICK) && keg->HasAura(SNT_SPELL_KEG_ACTIVE);
    });
}

Unit* FindTarTarget(Player* bot)
{
    if (!bot || !bot->IsAlive())
        return nullptr;

    Group* group = bot->GetGroup();
    auto const clear = [bot, group](Creature* creature, float radius) {
        if (!group)
            return bot->GetExactDist2d(creature) > radius;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->IsInMap(creature) && member->GetExactDist2d(creature) <= radius)
                return false;
        }

        return true;
    };

    // Vo'jak and the Warriors first (the tar's +10% damage taken pays most on them), then the Swarmers, then a Demolisher
    // (the tar's tick sets off its Unstable Blast, so nobody within 16 yd of it)
    static uint32 const entries[] = {SNT_NPC_VOJAK, SNT_NPC_SIKTHIK_WARRIOR, SNT_NPC_SIKTHIK_SWARMER, SNT_NPC_SIKTHIK_DEMOLISHER};
    for (uint32 const entry : entries)
    {
        float const radius = entry == SNT_NPC_SIKTHIK_DEMOLISHER ? SNT_DEMOLISHER_BLAST + 1.0f : SNT_TAR_SAFE;
        Creature* target = NearestCreature(bot, entry, 38.0f, [bot, &clear, radius](Creature* creature) {
            return !creature->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) && bot->IsWithinLOSInMap(creature) &&
                   AttackersValue::IsValidTarget(creature, bot) && clear(creature, radius);
        });
        if (target)
            return target;
    }

    return nullptr;
}

uint32 CastTimeLeftMs(Unit* caster, uint32 spellId)
{
    if (!caster || !CurrentCast(caster, spellId))
        return 0;

    // Gusting Winds is a channel whose aura (121282, 4000 ms, 5.4.8 SpellMisc.dbc duration index 35) sits on the caster
    if (Aura* aura = caster->GetAura(spellId))
        return aura->GetDuration() > 0 ? uint32(aura->GetDuration()) : 0;

    if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
        return info->GetDuration() > 0 ? uint32(info->GetDuration()) : 0;

    return 0;
}

bool CanReachBeforeCastEnds(Player* bot, Unit* caster, uint32 spellId, float range)
{
    if (!bot || !caster || range <= 0.0f)
        return false;

    // #656 w50: the 3 Gusting Winds channels of w50 started with the bots 80+ yd away (he lands at the far end of the
    // bridge); 4 s at ~7-8 yd/s covers ~30 yd, so they FAILED 36 chases in a row that could never arrive in time (the
    // bridge approach walks there anyway; the next channel comes 10 s later, boss_wing_leader_neronok.cpp:229-235).
    float const gap = bot->GetExactDist(caster) - range;
    if (gap <= 0.0f)
        return true;

    float const speed = std::max(bot->GetSpeed(MOVE_RUN), 1.0f);
    return gap / speed * 1000.0f <= float(CastTimeLeftMs(caster, spellId));
}

uint32 BombClickableLeftMs(Creature* bomb)
{
    // the bomb is a 20000 ms summon of 119377 (5.4.8 SpellMisc.dbc duration index 1): its age is 20000 - the summon timer
    TempSummon* summon = bomb ? bomb->ToTempSummon() : nullptr;
    if (!summon)
        return SNT_BOMB_ARM_MS;

    uint32 const timer = summon->GetTimer();
    if (timer == 0 || timer > SNT_BOMB_SUMMON_MS)
        return SNT_BOMB_ARM_MS;

    uint32 const age = SNT_BOMB_SUMMON_MS - timer;
    return age >= SNT_BOMB_ARM_MS ? 0 : SNT_BOMB_ARM_MS - age;
}

Creature* FindSapPuddle(Player* bot)
{
    Creature* jinbak = bot->FindNearestCreature(SNT_NPC_JINBAK, 70.0f);
    if (!jinbak || !jinbak->IsInCombat())
        return nullptr;

    return bot->FindNearestCreature(SNT_NPC_SAP_PUDDLE, 70.0f);
}

float SapPuddleRadius(Creature* puddle)
{
    return SNT_PUDDLE_BASE_RADIUS * std::max(puddle->GetObjectScale(), 0.1f);
}

uint32 SapGrowStacks(Creature* puddle)
{
    if (Aura* grow = puddle->GetAura(SNT_SPELL_SAP_GROW))
        return std::max<uint32>(grow->GetStackAmount(), puddle->GetAuraCount(SNT_SPELL_SAP_GROW));

    return 0;
}

uint32 SapResidueStacks(Player* bot)
{
    if (Aura* residue = bot->GetAura(SNT_SPELL_SAP_RESIDUE_DOT))
        return std::max<uint32>(residue->GetStackAmount(), bot->GetAuraCount(SNT_SPELL_SAP_RESIDUE_DOT));

    return 0;
}

bool WantsToSoakPuddle(Player* bot, Creature*& puddle)
{
    puddle = nullptr;
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return false;

    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return false;

    if (bot->GetHealthPct() < SNT_SOAK_MIN_HEALTH || SapResidueStacks(bot) + 1 >= SNT_RESIDUE_MAX)
        return false;

    // the Sap Globules die first (the focus value names them)
    if (bot->FindNearestCreature(SNT_NPC_SAP_GLOBULE, 60.0f))
        return false;

    Creature* found = FindSapPuddle(bot);
    if (!found || SapGrowStacks(found) < SNT_SOAK_MIN_GROW)
        return false;

    // already inside (the inner 60%): nothing to do
    if (bot->GetExactDist2d(found) <= SapPuddleRadius(found) * 0.6f)
        return false;

    puddle = found;
    return true;
}
}  // namespace MopSnt
