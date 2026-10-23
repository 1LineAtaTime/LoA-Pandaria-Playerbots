/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - hazard / focus / interrupt computations.
// See MopMspShared.h for the sources of every mechanic and id.

#include "MopMspShared.h"

#include "AttackersValue.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <list>
#include <string>
#include <vector>

namespace MopMsp
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

Creature* NearestCreature(Player* bot, uint32 entry, float range)
{
    return NearestCreature(bot, entry, range, [](Creature*) { return true; });
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

bool ConeHazard(Player* bot, Unit* caster, float halfAngle, float halfWidth, float length, float margin,
                HazardKind kind, Hazard& out)
{
    float ex, ey;
    if (!EscapeFromArea(caster->GetPositionX(), caster->GetPositionY(), caster->GetOrientation(), halfAngle,
                        halfWidth, length, margin, bot->GetPositionX(), bot->GetPositionY(), ex, ey))
        return false;

    out.kind = kind;
    out.x = ex;
    out.y = ey;
    return true;
}

// Xin's Ring of Fire: the trigger creature circles a 5 yd ring around its start point and then explodes in the
// middle (119590, knock up). Staying 10.5 yd from the creature keeps the bot outside the whole disc.
bool FireRingHazard(Player* bot, Hazard& out)
{
    Creature* ring = NearestCreature(bot, MSP_NPC_RING_OF_FIRE, MSP_HAZARD_SCAN);
    if (!ring)
        return false;

    return SetAway(bot, ring->GetPositionX(), ring->GetPositionY(), MSP_RING_SAFE_DIST, HazardKind::FireRing, out);
}

// Xin's Ground Slam: 3 s cast, 30 yd cone on the tank (armor -25%, stacks). Everybody but the victim leaves the cone.
bool GroundSlamHazard(Player* bot, Hazard& out)
{
    Creature* xin = bot->FindNearestCreature(MSP_NPC_XIN_THE_WEAPONMASTER, 70.0f);
    if (!xin || !CurrentCast(xin, MSP_SPELL_GROUND_SLAM))
        return false;

    if (xin->GetVictim() == bot)
        return false;

    return ConeHazard(bot, xin, float(M_PI) / 4.0f, 1.0f, 30.0f, 2.0f, HazardKind::GroundSlam, out);
}

// Kuai's Shockwave: a growing cone (60 yd) in the direction of the picked player, knocks everything in it into the
// air. The cone half angle is not in the data we have, 30 degrees plus margin covers the picked player's line.
bool ShockwaveHazard(Player* bot, Hazard& out)
{
    Creature* kuai = bot->FindNearestCreature(MSP_NPC_KUAI_THE_BRUTE, 90.0f);
    if (!kuai)
        return false;

    // the 2 s cast, then the aura on Kuai while the cone keeps growing (boss_trial_of_the_king.cpp:1826-1849)
    if (!CurrentCast(kuai, MSP_SPELL_SHOCKWAVE) && !kuai->HasAura(MSP_SPELL_SHOCKWAVE))
        return false;

    return ConeHazard(bot, kuai, float(M_PI) / 6.0f, 3.0f, 64.0f, 2.0f, HazardKind::Shockwave, out);
}

// Xin's animated axes: whirlwind (119373) while they roam the room.
bool AxeHazard(Player* bot, Hazard& out)
{
    float const safe = ClampedRadius(MSP_SPELL_WHIRLWIND, MSP_AXE_FALLBACK_RADIUS, 6.0f, 12.0f) + 1.5f;
    Creature* axe = NearestCreature(bot, MSP_NPC_ANIMATED_AXE, MSP_HAZARD_SCAN,
                                    [](Creature* creature) { return creature->HasAura(MSP_SPELL_WHIRLWIND); });
    if (!axe)
        return false;

    return SetAway(bot, axe->GetPositionX(), axe->GetPositionY(), safe, HazardKind::Axe, out);
}

// Ming's Whirling Dervish: a tornado that roams the room and throws everything it touches.
bool DervishHazard(Player* bot, Hazard& out)
{
    Creature* dervish = NearestCreature(bot, MSP_NPC_WHIRLING_DERVISH, MSP_HAZARD_SCAN, [](Creature* creature) {
        return creature->HasAura(MSP_SPELL_WHIRLING_DERVISH_AURA);
    });
    if (!dervish)
        return false;

    return SetAway(bot, dervish->GetPositionX(), dervish->GetPositionY(), MSP_DERVISH_SAFE_DIST,
                   HazardKind::Dervish, out);
}

// Haiyan's Conflagrate: the debuffed player (or anybody standing next to one) spreads the DoT at melee range, so the
// carrier steps away from the group and the others step away from the carrier.
bool ConflagrateHazard(Player* bot, Hazard& out)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const botHasIt = bot->HasAura(MSP_SPELL_CONFLAGRATE_AURA);
    Player* nearest = nullptr;
    float nearestDist = MSP_CONFLAGRATE_SPREAD;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!botHasIt && !member->HasAura(MSP_SPELL_CONFLAGRATE_AURA))
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist < nearestDist)
        {
            nearest = member;
            nearestDist = dist;
        }
    }

    if (!nearest)
        return false;

    return SetAway(bot, nearest->GetPositionX(), nearest->GetPositionY(), MSP_CONFLAGRATE_SPREAD + 1.0f,
                   HazardKind::Conflagrate, out);
}

// Haiyan's Meteor: 800000 Fire divided amongst everyone within 10 yd of the impact, so the group stacks on the
// targeted player (who stands still) while the 4 s cast runs.
bool MeteorStackHazard(Player* bot, Hazard& out)
{
    Creature* haiyan = bot->FindNearestCreature(MSP_NPC_HAIYAN_THE_UNSTOPPABLE, 90.0f);
    Spell* meteor = CurrentCast(haiyan, MSP_SPELL_METEOR);
    if (!meteor)
        return false;

    ObjectGuid const targetGuid = meteor->m_targets.GetUnitTargetGUID();
    if (targetGuid.IsEmpty() || targetGuid == bot->GetGUID())
        return false;

    Unit* target = ObjectAccessor::GetUnit(*bot, targetGuid);
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
        return false;

    float const dist = bot->GetExactDist2d(target);
    if (dist <= MSP_METEOR_STACK_DIST)
        return false;

    float const ux = (bot->GetPositionX() - target->GetPositionX()) / dist;
    float const uy = (bot->GetPositionY() - target->GetPositionY()) / dist;
    out.kind = HazardKind::MeteorStack;
    out.x = target->GetPositionX() + ux * 3.0f;
    out.y = target->GetPositionY() + uy * 3.0f;
    return true;
}

// #654 w47: a healer more than MSP_HEALER_LEASH_MAX from Xin's victim, or without LOS to it (the dais), walks back toward
// it. Lowest priority of all hazards (ComputeHazard); AC shape: Dungeon/Mech/MechActions.cpp:216-218 (healer leashed to
// the tank while dodging).
bool HealerLeashHazard(Player* bot, Hazard& out)
{
    Unit* tank = HealerLeashTarget(bot);
    if (!tank)
        return false;

    float const dist = bot->GetExactDist2d(tank);
    if (dist < 1.0f || (dist <= MSP_HEALER_LEASH_MAX && bot->IsWithinLOSInMap(tank)))
        return false;

    float const keep = std::min(MSP_HEALER_LEASH_DIST, dist * 0.5f);
    out.kind = HazardKind::HealerLeash;
    out.x = tank->GetPositionX() + (bot->GetPositionX() - tank->GetPositionX()) / dist * keep;
    out.y = tank->GetPositionY() + (bot->GetPositionY() - tank->GetPositionY()) / dist * keep;
    return true;
}

struct InterruptRule
{
    uint32 entry;
    uint32 spellId;
};

// Ming's Lightning Bolt (3 s cast, 277500 Nature on the tank).
InterruptRule const MING_RULES[] = {{MSP_NPC_MING_THE_CUNNING, MSP_SPELL_LIGHTNING_BOLT}};

// Gekkan's casters: the Oracle's Cleansing Flame (raid heal of ~1.2M) first, the Hexxer's Hex of Lethargy second.
InterruptRule const GEKKAN_RULES[] = {{MSP_NPC_GLINTROK_ORACLE, MSP_SPELL_CLEANSING_FLAME},
                                      {MSP_NPC_GLINTROK_HEXXER, MSP_SPELL_HEX_OF_LETHARGY}};

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

    return FireRingHazard(bot, out) || GroundSlamHazard(bot, out) || ShockwaveHazard(bot, out) ||
           AxeHazard(bot, out) || DervishHazard(bot, out) || ConflagrateHazard(bot, out) ||
           MeteorStackHazard(bot, out) || HealerLeashHazard(bot, out);
}

Unit* HealerLeashTarget(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || !PlayerBotSpec::IsHeal(bot))
        return nullptr;

    Group* group = bot->GetGroup();
    Creature* xin = bot->FindNearestCreature(MSP_NPC_XIN_THE_WEAPONMASTER, 90.0f);
    if (!group || !xin || !xin->IsInCombat())
        return nullptr;

    Unit* victim = xin->GetVictim();
    if (!victim || victim == bot || !victim->IsAlive() || victim->GetTypeId() != TYPEID_PLAYER ||
        !group->IsMember(victim->GetGUID()))
        return nullptr;

    return victim;
}

ObjectGuid ComputeFocusTarget(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return ObjectGuid::Empty;

    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return ObjectGuid::Empty;

    // Only inside Gekkan's room: his protectors are the only focus order of this dungeon.
    if (!bot->FindNearestCreature(MSP_NPC_GEKKAN, 80.0f))
        return ObjectGuid::Empty;

    static uint32 const order[] = {MSP_NPC_GLINTROK_ORACLE, MSP_NPC_GLINTROK_HEXXER, MSP_NPC_GLINTROK_SKULKER,
                                   MSP_NPC_GLINTROK_IRONHIDE};
    for (uint32 const entry : order)
    {
        Creature* protector = NearestCreature(bot, entry, 45.0f, [bot](Creature* creature) {
            return creature->IsInCombat() && AttackersValue::IsValidTarget(creature, bot);
        });
        if (protector)
            return protector->GetGUID();
    }

    return ObjectGuid::Empty;
}

GuidVector ComputeExclusions(Player* bot)
{
    GuidVector guids;
    if (!bot || !bot->IsInCombat())
        return guids;

    std::list<Creature*> pets;
    bot->GetCreatureListWithEntryInGrid(pets, MSP_NPC_MU_SHIBA, 60.0f);
    for (Creature* pet : pets)
    {
        if (pet)
            guids.push_back(pet->GetGUID());
    }

    return guids;
}

Unit* FindInterruptTarget(Player* bot, InterruptGroup group)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return nullptr;

    InterruptRule const* rules = group == InterruptGroup::Ming ? MING_RULES : GEKKAN_RULES;
    size_t const count = group == InterruptGroup::Ming ? std::size(MING_RULES) : std::size(GEKKAN_RULES);
    for (size_t i = 0; i < count; ++i)
    {
        uint32 const spellId = rules[i].spellId;
        Creature* caster = NearestCreature(bot, rules[i].entry, 40.0f, [spellId](Creature* creature) {
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
}  // namespace MopMsp
