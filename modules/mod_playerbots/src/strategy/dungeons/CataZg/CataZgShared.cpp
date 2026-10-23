/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) shared helpers, MoP-only lane L7 (#670). See CataZgShared.h for sources.

#include "CataZgShared.h"
#include "AttackersValue.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "Group.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <list>
#include <string>

namespace CataZulGurub
{
namespace
{
constexpr float DEADZONE_SCAN_RANGE = 60.0f;

float AngleDiff(float a, float b)
{
    float const twoPi = 2.0f * static_cast<float>(M_PI);
    float d = std::fmod(a - b, twoPi);
    if (d > static_cast<float>(M_PI))
        d -= twoPi;
    else if (d < -static_cast<float>(M_PI))
        d += twoPi;

    return std::fabs(d);
}

Hazard Circle(float x, float y, float radius)
{
    return { HazardShape::Circle, x, y, radius, 0.0f, 0.0f };
}

Hazard Cone(float x, float y, float length, float angle, float halfAngle)
{
    return { HazardShape::Cone, x, y, length, angle, halfAngle };
}

Hazard Line(float x, float y, float length, float angle, float halfWidth)
{
    return { HazardShape::Line, x, y, length, angle, halfWidth };
}

// Nearest valid, attackable creature of an entry that the optional filter accepts (MgTShared.cpp
// GetActiveFelCrystal gate: AttackersValue::IsValidTarget).
Unit* NearestAttackable(Player* bot, uint32 entry, float range, bool (*accept)(Creature*) = nullptr)
{
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, entry, range);

    Creature* best = nullptr;
    float bestDist = range + 1.0f;
    for (Creature* creature : found)
    {
        if (!creature->IsAlive() || (accept && !accept(creature)))
            continue;

        if (!AttackersValue::IsValidTarget(creature, bot))
            continue;

        float const dist = bot->GetExactDist2d(creature);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = creature;
        }
    }

    return best;
}

bool OhganIsFighting(Creature* ohgan)
{
    return !ohgan->HasAura(SPELL_OHGAN_FEIGN_DEATH);
}

bool PantherIsAwake(Creature* panther)
{
    return !panther->HasAura(SPELL_DARK_SLUMBER);
}

bool ChainIsExposed(Creature* chain)
{
    return !chain->HasAura(SPELL_BRITTLE_BARRIER);
}

bool TwistedSpiritIsFighting(Creature* spirit)
{
    return spirit->IsInCombat();
}

// Every living creature of an entry within range.
std::list<Creature*> LivingInGrid(Player* bot, uint32 entry, float range)
{
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, entry, range);
    found.remove_if([](Creature* creature) { return !creature->IsAlive(); });
    return found;
}
}

float HazardSlack(Hazard const& hazard, float x, float y)
{
    float const dx = x - hazard.x;
    float const dy = y - hazard.y;
    float const dist = std::hypot(dx, dy);

    if (hazard.shape == HazardShape::Circle)
        return dist - hazard.radius;

    if (hazard.shape == HazardShape::Line)
    {
        // Distance to the segment [origin, origin + length * (cos, sin)] minus the half width; behind the origin (the
        // caster's back) is outside.
        float const ux = std::cos(hazard.angle);
        float const uy = std::sin(hazard.angle);
        float const along = dx * ux + dy * uy;
        if (along < 0.0f)
            return dist;

        float const t = std::min(along, hazard.radius);
        return std::hypot(dx - ux * t, dy - uy * t) - hazard.halfAngle;
    }

    // Cone: outside once past its length, otherwise by the angular distance to its nearest side.
    if (dist > hazard.radius)
        return dist - hazard.radius;

    if (dist < 0.001f)
        return -1.0f;

    float const diff = AngleDiff(std::atan2(dy, dx), hazard.angle);
    if (diff <= hazard.halfAngle)
        return -1.0f;

    float const over = diff - hazard.halfAngle;
    if (over >= static_cast<float>(M_PI_2))
        return dist;

    return dist * std::sin(over);
}

float MinSlack(std::vector<Hazard> const& hazards, float x, float y)
{
    float slack = 1000.0f;
    for (Hazard const& hazard : hazards)
        slack = std::min(slack, HazardSlack(hazard, x, y));

    return slack;
}

// 5.4.8: SpellInfo::Effects[] / SpellEffectInfo::CalcRadius(), MAX_SPELL_EFFECTS - same use as UBShared.cpp MaxEffectRadius.
float SpellRadius(uint32 spellId, float fallback)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return fallback;

    float radius = 0.0f;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        radius = std::max(radius, spellInfo->Effects[i].CalcRadius());

    return radius > 0.0f ? radius : fallback;
}

Creature* FindBoss(Player* bot, uint32 entry)
{
    return bot->FindNearestCreature(entry, BOSS_SCAN_RANGE, true);
}

bool IsShadowsOfHakkarActive(Player* bot)
{
    Creature* jindo = FindBoss(bot, NPC_JINDO);
    if (!jindo)
        return false;

    if (jindo->HasAura(SPELL_SHADOWS_OF_HAKKAR))
        return true;

    return jindo->HasUnitState(UNIT_STATE_CASTING) && jindo->FindCurrentSpellBySpellId(SPELL_SHADOWS_OF_HAKKAR);
}

void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB)
        return;

    switch (kind)
    {
        case HazardKind::AcridPools:
        {
            // Venoxis phase 2: Pool of Acrid Tears 52197 grows over time; the damage spell reaches 2 * scale yd
            // (boss_venoxis.cpp:518-535 spell_venoxis_pool_of_acid_tears_dmg).
            if (!FindBoss(bot, NPC_VENOXIS))
                return;

            std::list<Creature*> pools;
            bot->GetCreatureListWithEntryInGrid(pools, NPC_POOL_OF_ACRID_TEARS, HAZARD_SCAN_RANGE);
            for (Creature* pool : pools)
            {
                if (pool->IsAlive())
                {
                    out.push_back(Circle(pool->GetPositionX(), pool->GetPositionY(),
                                         POOL_BASE_RADIUS * pool->GetObjectScale() + 0.5f));
                }
            }

            return;
        }
        case HazardKind::Bloodvenom:
        {
            // Venoxis phase 3: one Bloodvenom tendril per player, slow (run speed 0.3) and area damaging.
            std::list<Creature*> tendrils;
            bot->GetCreatureListWithEntryInGrid(tendrils, NPC_BLOODVENOM, HAZARD_SCAN_RANGE);
            for (Creature* tendril : tendrils)
            {
                if (tendril->IsAlive())
                    out.push_back(Circle(tendril->GetPositionX(), tendril->GetPositionY(), BLOODVENOM_RADIUS));
            }

            return;
        }
        case HazardKind::DevastatingSlam:
        {
            // Mandokir turns to a non-tank player, casts Devastating Slam (2 s) and the script drops 50 missiles in a
            // 35 degree cone out to ~54 yd (boss_mandokir.cpp:555-566).
            Creature* boss = FindBoss(bot, NPC_MANDOKIR);
            if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_DEVASTATING_SLAM))
                return;

            out.push_back(Cone(boss->GetPositionX(), boss->GetPositionY(), SLAM_LENGTH, boss->GetOrientation(),
                               SLAM_HALF_ANGLE));
            return;
        }
        case HazardKind::WaveOfAgony:
        {
            // Kilnara marks a stalker (52160) and 3.8 s later the wave sweeps from the boss toward it
            // (boss_kilnara.cpp:97-100, 363-385; spell 96460 is a 70 yd cone).
            Creature* boss = FindBoss(bot, NPC_KILNARA);
            if (!boss)
                return;

            std::list<Creature*> marks;
            bot->GetCreatureListWithEntryInGrid(marks, NPC_WAVE_OF_AGONY_END, HAZARD_SCAN_RANGE);
            for (Creature* mark : marks)
            {
                if (mark->IsAlive())
                {
                    out.push_back(Cone(boss->GetPositionX(), boss->GetPositionY(), WAVE_LENGTH, boss->GetAngle(mark),
                                       WAVE_HALF_ANGLE));
                }
            }

            return;
        }
        case HazardKind::ZanzilFire:
        {
            // Zanzili Fire lays a 4 yd fire every 4 yd in front of the boss (boss_zanzil.cpp:288-305).
            if (!FindBoss(bot, NPC_ZANZIL))
                return;

            float const radius = SpellRadius(SPELL_ZANZIL_FIRE_TRAIL, 4.0f);
            for (Position const& pos :
                 EncounterHelpers::GetDynamicObjectPositions(bot, HAZARD_SCAN_RANGE, SPELL_ZANZIL_FIRE_TRAIL))
            {
                out.push_back(Circle(pos.GetPositionX(), pos.GetPositionY(), radius));
            }

            // #670 w61: while Zanzil casts Zanzili Fire (1.5 s, at his victim - the tank) the whole line in front of him is
            // the hazard, so the tank side-steps before the first 40000 hit instead of after it. Wowpedia "Zanzil (tactics)":
            // the trail moves away from Zanzil in the direction he faced when the cast started.
            Creature* zanzil = FindBoss(bot, NPC_ZANZIL);
            if (zanzil && zanzil->IsNonMeleeSpellCasted(true) && zanzil->FindCurrentSpellBySpellId(SPELL_ZANZIL_FIRE_CAST))
            {
                out.push_back(Line(zanzil->GetPositionX(), zanzil->GetPositionY(), ZANZIL_FIRE_LINE_LENGTH,
                                   zanzil->GetOrientation(), ZANZIL_FIRE_LINE_HALF_WIDTH));
            }

            return;
        }
        case HazardKind::Deadzone:
        {
            // Phase 1: a Deadzone cuts cast speed by 90%, so casters and the boss stay out of it - except while
            // Shadows of Hakkar is up, when everybody stands in one (see GetDeadzoneToEnter).
            if (!FindBoss(bot, NPC_JINDO) || IsShadowsOfHakkarActive(bot))
                return;

            float const radius = SpellRadius(SPELL_DEADZONE, 5.0f);
            for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, HAZARD_SCAN_RANGE, SPELL_DEADZONE))
                out.push_back(Circle(pos.GetPositionX(), pos.GetPositionY(), radius));

            return;
        }
        case HazardKind::BreathOfHethiss:
        {
            // #670 w57/w60: Venoxis wiped the group in phase 2. Breath of Hethiss (boss_venoxis.cpp:221-223, every 10 s once
            // he transforms) is a 1.5 s cast, then a 3 s self aura that stuns him in place and pulses 96744 every 0.5 s:
            // 30000 nature (heroic, SpellEffect.dbc diff 2) to every enemy in a 15 yd cone in front - 180k over the channel.
            // Wowpedia "High Priest Venoxis (tactics)": the tank and anyone within 15 yd must be behind him before it lands.
            Creature* boss = FindBoss(bot, NPC_VENOXIS);
            if (!boss)
                return;

            bool const casting = boss->IsNonMeleeSpellCasted(true) && boss->FindCurrentSpellBySpellId(SPELL_BREATH_OF_HETHISS);
            if (!casting && !boss->HasAura(SPELL_BREATH_OF_HETHISS))
                return;

            out.push_back(Cone(boss->GetPositionX(), boss->GetPositionY(), BREATH_LENGTH, boss->GetOrientation(),
                               BREATH_HALF_ANGLE));
            return;
        }
        case HazardKind::BerserkerPursuit:
        {
            // A Zanzili Berserker fixates one random player (Pursuit, boss_zanzil.cpp:254-258) and walks at half speed; the
            // pursued player kites it (Wowpedia "Zanzil (tactics)"). The tank keeps its place in front of Zanzil.
            // #670 w61: a pursued melee DPS kited instead of hitting it (Carino, rogue: "zg kite berserker" while the focus target
            // was the berserker - no melee uptime on the add everybody focuses); melee DPS stand and fight it.
            if (PlayerBotSpec::IsTank(bot) || (PlayerBotSpec::IsMelee(bot) && PlayerBotSpec::IsDps(bot)))
                return;

            for (Creature* berserker : LivingInGrid(bot, NPC_ZANZILI_BERSERKER, HAZARD_SCAN_RANGE))
            {
                if (berserker->GetVictim() == bot)
                    out.push_back(Circle(berserker->GetPositionX(), berserker->GetPositionY(), BERSERKER_KITE_RADIUS));
            }

            return;
        }
        case HazardKind::PoisonCloud:
        {
            // #670 w68: every bot that died to Venoxis died on his altar / stairs inside 1-3 overlapping Poison Clouds (see
            // CataZgShared.h). 7 yd + this bot's combat reach (the spell's IsWithinDist3d); a bunny more than that above or below
            // the bot cannot reach it.
            if (!FindBoss(bot, NPC_VENOXIS))
                return;

            float const radius = SpellRadius(SPELL_POISON_CLOUD_DMG, 7.0f) + bot->GetObjectSize();
            for (Creature* bunny : LivingInGrid(bot, NPC_GENERAL_BUNNY_JMF, HAZARD_SCAN_RANGE))
            {
                if (bunny->HasAura(SPELL_POISON_CLOUD_AURA) && std::fabs(bunny->GetPositionZ() - bot->GetPositionZ()) < radius)
                    out.push_back(Circle(bunny->GetPositionX(), bunny->GetPositionY(), radius));
            }

            return;
        }
        case HazardKind::VenomousEffusion:
        {
            // #679: every living Venomous Effusion cloud is a 2 yd circle (96685 SRC_AREA_ENEMY radius, + the bot's reach + a margin).
            if (!FindBoss(bot, NPC_VENOXIS))
                return;

            float const radius = SpellRadius(SPELL_VENOMOUS_EFFUSION_DMG, 2.0f) + bot->GetObjectSize() + EFFUSION_MARGIN;
            // #679 w105: the stalker lays a cloud every 0.5 s; a circle round it covers the wall of the next ~1.5 s (reactive-only avoid left 10 hits)
            for (Creature* stalker : LivingInGrid(bot, NPC_VENOMOUS_EFFUSION_STALKER, HAZARD_SCAN_RANGE))
            {
                if (std::fabs(stalker->GetPositionZ() - bot->GetPositionZ()) < 6.0f)
                    out.push_back(Circle(stalker->GetPositionX(), stalker->GetPositionY(), EFFUSION_STALKER_RADIUS + bot->GetObjectSize()));
            }
            for (Creature* cloud : LivingInGrid(bot, NPC_VENOMOUS_EFFUSION, HAZARD_SCAN_RANGE))
            {
                if (std::fabs(cloud->GetPositionZ() - bot->GetPositionZ()) < 6.0f)
                    out.push_back(Circle(cloud->GetPositionX(), cloud->GetPositionY(), radius));
            }

            return;
        }
    }
}

// #679 w106: the straight walk from (x0, y0) to (x1, y1) enters no hazard the walker is not already standing in (sampled every 1 yd;
// a hazard it stands in can only be left). w106: the mage's Acrid Pools escape and the rogue's Poison Cloud escape ended clear of the
// Venomous Effusion walls but walked through them (9 + 2 of the 13 hits).
bool PathClear(std::vector<Hazard> const& hazards, float x0, float y0, float x1, float y1)
{
    std::vector<Hazard> crossing;
    for (Hazard const& hazard : hazards)
    {
        if (HazardSlack(hazard, x0, y0) >= 0.0f)
            crossing.push_back(hazard);
    }

    if (crossing.empty())
        return true;

    float const len = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    int const steps = std::max(1, static_cast<int>(std::ceil(len)));
    for (int i = 1; i < steps; ++i)
    {
        float const t = static_cast<float>(i) / static_cast<float>(steps);
        if (MinSlack(crossing, x0 + (x1 - x0) * t, y0 + (y1 - y0) * t) < 0.0f)
            return false;
    }

    return true;
}

bool FindEscapeSpot(Player* bot, std::vector<Hazard> const& hazards, float margin, float& outX, float& outY,
                    float& outZ, bool clearPath)
{
    constexpr float RADII[] = { 3.0f, 5.0f, 7.0f, 9.0f, 12.0f, 15.0f, 19.0f, 24.0f };
    constexpr int BEARINGS = 16;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();

    struct Candidate
    {
        float slack;
        float x;
        float y;
    };

    // First pass wants the full margin everywhere; the second settles for merely being outside every hazard.
    for (float const required : { margin, 0.0f })
    {
        for (float const radius : RADII)
        {
            std::vector<Candidate> ring;
            for (int i = 0; i < BEARINGS; ++i)
            {
                float const angle = (2.0f * static_cast<float>(M_PI) * i) / BEARINGS;
                float const x = bx + std::cos(angle) * radius;
                float const y = by + std::sin(angle) * radius;
                float const slack = MinSlack(hazards, x, y);
                if (slack >= required)
                    ring.push_back({ std::min(slack, margin + 3.0f), x, y });
            }

            std::stable_sort(ring.begin(), ring.end(),
                             [](Candidate const& a, Candidate const& b) { return a.slack > b.slack; });

            for (Candidate const& candidate : ring)
            {
                float const z = bot->GetMapHeight(candidate.x, candidate.y, bz + 5.0f, true, 12.0f);
                if (!(std::fabs(z - bz) <= 3.0f))
                    continue;

                if (!bot->IsWithinLOS(candidate.x, candidate.y, z + 2.0f))
                    continue;

                if (clearPath && !PathClear(hazards, bx, by, candidate.x, candidate.y))
                    continue;

                outX = candidate.x;
                outY = candidate.y;
                outZ = z;
                return true;
            }
        }
    }

    return false;
}

Player* GetToxicPartner(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->HasAura(SPELL_TOXIC_LINK_AURA))
        return nullptr;

    // #670 w67: the tank (linked with the rogue) ran 9 yd west for the link and dragged Venoxis to (-12031,-1701.7), out of the
    // healer's line of sight and 43 yd from it; the tank died 23 s into the pull. The tank holds the boss; its partner breaks
    // the link alone (the action already moves the full remaining distance, 10 yd per step).
    if (PlayerBotSpec::IsTank(bot))
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId() ||
            !member->HasAura(SPELL_TOXIC_LINK_AURA))
            continue;

        return bot->GetDistance2d(member) < TOXIC_LINK_BREAK_DISTANCE ? member : nullptr;
    }

    return nullptr;
}

bool GetVenoxisTankSpot(Player* bot, float& x, float& y, float& z)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat() || !PlayerBotSpec::IsTank(bot))
        return false;

    Creature* boss = FindBoss(bot, NPC_VENOXIS);
    if (!boss || !boss->IsInCombat() || boss->GetVictim() != bot)
        return false;

    // Breath of Hethiss: the tank steps behind him first ("zg escape breath of hethiss"), then comes back.
    if ((boss->IsNonMeleeSpellCasted(true) && boss->FindCurrentSpellBySpellId(SPELL_BREATH_OF_HETHISS)) ||
        boss->HasAura(SPELL_BREATH_OF_HETHISS))
        return false;

    if (bot->GetExactDist2d(VENOXIS_TANK_X, VENOXIS_TANK_Y) <= VENOXIS_TANK_SLACK)
        return false;

    // A Pool of Acrid Tears grew over the spot: hold him where he is ("zg escape acrid pools" moves the tank out of it).
    // #679: the same for a Venomous Effusion wall across the spot.
    std::vector<Hazard> pools;
    CollectHazards(bot, HazardKind::AcridPools, pools);
    CollectHazards(bot, HazardKind::VenomousEffusion, pools);
    if (!pools.empty() && MinSlack(pools, VENOXIS_TANK_X, VENOXIS_TANK_Y) < CLEAR_MARGIN)
        return false;

    x = VENOXIS_TANK_X;
    y = VENOXIS_TANK_Y;
    z = VENOXIS_TANK_Z;
    return true;
}

bool IsVenoxisInPoisonCloud(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat())
        return false;

    Creature* boss = FindBoss(bot, NPC_VENOXIS);
    if (!boss)
        return false;

    std::vector<Hazard> clouds;
    CollectHazards(bot, HazardKind::PoisonCloud, clouds);
    return !clouds.empty() && MinSlack(clouds, boss->GetPositionX(), boss->GetPositionY()) < 0.0f;
}

bool GetDeadzoneToEnter(Player* bot, float& centreX, float& centreY, float& radius)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat() || bot->HasAura(SPELL_DEADZONE) ||
        !IsShadowsOfHakkarActive(bot))
        return false;

    float bestDist = 0.0f;
    bool found = false;
    for (Position const& zone : EncounterHelpers::GetDynamicObjectPositions(bot, DEADZONE_SCAN_RANGE, SPELL_DEADZONE))
    {
        float const dist = bot->GetExactDist2d(zone);
        if (!found || dist < bestDist)
        {
            found = true;
            bestDist = dist;
            centreX = zone.GetPositionX();
            centreY = zone.GetPositionY();
        }
    }

    if (!found)
        return false;

    radius = SpellRadius(SPELL_DEADZONE, 5.0f);
    return true;
}

// Jin'do phase 2. At 70% Jin'do turns unattackable and summons three Hakkar's Chains (52430, creature_template_addon
// auras 97091 + 97417 Brittle Barrier, boss_jindo_the_godbreaker.cpp:214-221). Only a Gurubashi Spirit's Body Slam removes a
// barrier (npc_jindo_gurubashi_spirit MovementInform, :368-373), and Jin'do dies once all three chains are dead (:185-206).
Creature* GetShieldedChain(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB)
        return nullptr;

    Creature* best = nullptr;
    for (Creature* chain : LivingInGrid(bot, NPC_HAKKAR_CHAINS, SPIRIT_SCAN_RANGE))
    {
        if (ChainIsExposed(chain))
            continue;

        if (!best || chain->GetGUID().GetCounter() < best->GetGUID().GetCounter())
            best = chain;
    }

    return best;
}

bool HasExposedChain(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB)
        return false;

    for (Creature* chain : LivingInGrid(bot, NPC_HAKKAR_CHAINS, SPIRIT_SCAN_RANGE))
    {
        if (ChainIsExposed(chain))
            return true;
    }

    return false;
}

Creature* GetEngagedSpirit(Player* bot)
{
    for (Creature* spirit : LivingInGrid(bot, NPC_GURUBASHI_SPIRIT, SPIRIT_SCAN_RANGE))
    {
        if (spirit->IsInCombat())
            return spirit;
    }

    return nullptr;
}

Creature* GetIdleSpirit(Player* bot)
{
    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* spirit : LivingInGrid(bot, NPC_GURUBASHI_SPIRIT, SPIRIT_SCAN_RANGE))
    {
        if (spirit->IsInCombat())
            continue;

        float const dist = bot->GetExactDist2d(spirit);
        if (!best || dist < bestDist)
        {
            best = spirit;
            bestDist = dist;
        }
    }

    return best;
}

bool IsSpiritPuller(Player* bot)
{
    if (PlayerBotSpec::IsTank(bot))
        return true;

    if (!PlayerBotSpec::IsDps(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    // Fallback when the tank is dead: the living DPS bot with the lowest GUID counter (the same answer on every bot).
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId() ||
            !GET_PLAYERBOT_AI(member))
            continue;

        if (PlayerBotSpec::IsTank(member))
            return false;

        if (PlayerBotSpec::IsDps(member) && member->GetGUID().GetCounter() < bot->GetGUID().GetCounter())
            return false;
    }

    return true;
}

Creature* GetSpiritToPull(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat() || !bot->IsAlive())
        return nullptr;

    if (!GetShieldedChain(bot) || HasExposedChain(bot) || GetEngagedSpirit(bot) || !IsSpiritPuller(bot))
        return nullptr;

    return GetIdleSpirit(bot);
}

Creature* GetChainToStandAt(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat() || !bot->IsAlive())
        return nullptr;

    // An exposed chain is killed where it stands; the group moves on to the next one afterwards.
    if (HasExposedChain(bot))
        return nullptr;

    Creature* chain = GetShieldedChain(bot);
    if (!chain)
        return nullptr;

    // The puller fetches a spirit first (it walks back to the chain once the spirit fights the group).
    if (GetSpiritToPull(bot))
        return nullptr;

    return chain;
}

bool IsGazeTarget(Player* bot)
{
    return bot->HasAura(SPELL_SPIRIT_WARRIOR_GAZE);
}

void GetChainWaitSpot(Player* bot, Creature* chain, float& x, float& y)
{
    // This bot's slot among the bots of its group (group order, the same answer on every bot; the master is not a bot).
    uint32 slot = 0;
    uint32 bots = 0;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !GET_PLAYERBOT_AI(member))
                continue;

            if (member == bot)
                slot = bots;

            ++bots;
        }
    }

    float const away = std::atan2(chain->GetPositionY() - HAKKAR_SPIRIT_Y, chain->GetPositionX() - HAKKAR_SPIRIT_X);
    float const offset = bots > 1 ? (float(slot) - float(bots - 1) / 2.0f) * CHAIN_WAIT_SPREAD : 0.0f;
    x = chain->GetPositionX() + std::cos(away + offset) * CHAIN_WAIT_DIST;
    y = chain->GetPositionY() + std::sin(away + offset) * CHAIN_WAIT_DIST;
}

bool IsChainPhase(Player* bot)
{
    return bot->GetMapId() == MAP_ZUL_GURUB && bot->IsInCombat() &&
           !LivingInGrid(bot, NPC_HAKKAR_CHAINS, SPIRIT_SCAN_RANGE).empty();
}

Unit* GetFocusTarget(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat())
        return nullptr;

    // Same role gate as MgTTriggers.cpp MgTFocusTargetTrigger: DPS only, never the tank or a healer.
    if (PlayerBotSpec::IsTank(bot))
    {
        // #670 w67: both DPS were dead when Jin'do's third chain (1433) lost its barrier, and the tank never attacked it (0
        // target ticks on 52430:1433 in 320 s; the chains are passive, so they never reach the tank's attackers list). Jin'do
        // only dies once all three chains are dead (boss_jindo_the_godbreaker.cpp:185-206), so with no living DPS bot the tank
        // takes the exposed chain itself.
        if (!FindBoss(bot, NPC_JINDO) && !bot->FindNearestCreature(NPC_HAKKAR_CHAINS, 80.0f, true))
            return nullptr;

        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive() && member->GetMapId() == bot->GetMapId() &&
                    GET_PLAYERBOT_AI(member) && PlayerBotSpec::IsDps(member) && !PlayerBotSpec::IsTank(member))
                    return nullptr;
            }
        }

        return NearestAttackable(bot, NPC_HAKKAR_CHAINS, 80.0f, &ChainIsExposed);
    }

    if (!PlayerBotSpec::IsDps(bot))
        return nullptr;

    // Mandokir: all DPS focus the raptor before it kills the Chained Spirits that resurrect the group.
    if (FindBoss(bot, NPC_MANDOKIR))
    {
        if (Unit* ohgan = NearestAttackable(bot, NPC_OHGAN, 80.0f, &OhganIsFighting))
            return ohgan;
    }

    // Kilnara: any panther still up when she turns (50%) makes the phase unhealable.
    if (Creature* kilnara = FindBoss(bot, NPC_KILNARA))
    {
        if (kilnara->GetHealthPct() <= 50.0f)
        {
            if (Unit* panther = NearestAttackable(bot, NPC_PRIDE_OF_BETHEKK, 60.0f, &PantherIsAwake))
                return panther;
        }
    }

    // Zanzil: kill the Zanzili Berserker before it fixates and reaches somebody.
    if (FindBoss(bot, NPC_ZANZIL))
    {
        if (Unit* berserker = NearestAttackable(bot, NPC_ZANZILI_BERSERKER, 70.0f))
            return berserker;
    }

    // Jin'do phase 2: a chain can only be damaged once its Brittle Barrier has been broken.
    if (FindBoss(bot, NPC_JINDO) || bot->FindNearestCreature(NPC_HAKKAR_CHAINS, 80.0f, true))
    {
        if (Unit* chain = NearestAttackable(bot, NPC_HAKKAR_CHAINS, 80.0f, &ChainIsExposed))
            return chain;

        // While every chain is still shielded the DPS stay off the Gurubashi Spirit (4 x 80195 hp, it would die before its
        // first Body Slam at 12 s) and only clear the Twisted Spirits that are fighting near them.
        if (GetShieldedChain(bot))
        {
            // #670 w65: the healer stood at 34% with 2-4 Twisted Spirits on it before the second slam. Wowpedia "Jin'do the
            // Godbreaker": "Ranged DPS should kill any spirits chasing the healer when no chains are available for attack".
            Unit* healerChaser = nullptr;
            float healerChaserDist = 0.0f;
            for (Creature* twisted : LivingInGrid(bot, NPC_TWISTED_SPIRIT, 40.0f))
            {
                Unit* victim = twisted->GetVictim();
                Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
                if (!victimPlayer || !PlayerBotSpec::IsHeal(victimPlayer) || !AttackersValue::IsValidTarget(twisted, bot))
                    continue;

                float const dist = bot->GetExactDist2d(twisted);
                if (!healerChaser || dist < healerChaserDist)
                {
                    healerChaser = twisted;
                    healerChaserDist = dist;
                }
            }

            if (healerChaser)
                return healerChaser;

            if (Unit* twisted = NearestAttackable(bot, NPC_TWISTED_SPIRIT, 30.0f, &TwistedSpiritIsFighting))
                return twisted;
        }
    }

    return nullptr;
}

// Zanzil cauldron (#822): the bots that may drink are the non-tank DPS.
static bool IsCauldronCandidate(Player* member)
{
    return member->IsAlive() && PlayerBotSpec::IsDps(member) && !PlayerBotSpec::IsTank(member);
}

// The bot that drinks: one that already carries the aura stays elected, else the lowest-GUID ranged DPS, else the lowest-GUID other
// DPS (the same answer on every bot). A bot a Berserker is fixated on is skipped (it is kiting).
static Player* ElectCauldronBot(Player* bot, Unit* berserker)
{
    Group* group = bot->GetGroup();
    if (!group)
        return IsCauldronCandidate(bot) ? bot : nullptr;

    Player* best = nullptr;
    bool bestRanged = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != bot->GetMapId() || !GET_PLAYERBOT_AI(member) || !IsCauldronCandidate(member))
            continue;

        if (member->HasAura(SPELL_FROSTBURN_AURA))
            return member;

        if (berserker->GetVictim() == member)
            continue;

        bool const ranged = PlayerBotSpec::IsRangedDps(member);
        if (!best || (ranged && !bestRanged) ||
            (ranged == bestRanged && member->GetGUID().GetCounter() < best->GetGUID().GetCounter()))
        {
            best = member;
            bestRanged = ranged;
        }
    }

    return best;
}

bool GroupHasFrostburnCarrier(Player* bot)
{
    if (bot->HasAura(SPELL_FROSTBURN_AURA))
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == bot->GetMapId() && member->HasAura(SPELL_FROSTBURN_AURA))
            return true;
    }

    return false;
}

bool GroupHasCauldronBuff(Player* bot)
{
    if (GroupHasFrostburnCarrier(bot))
        return true;

    if (bot->HasAura(SPELL_TOXIC_TORMENT))
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == bot->GetMapId() && member->HasAura(SPELL_TOXIC_TORMENT))
            return true;
    }

    return false;
}

bool IsGraveyardGasActive(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat())
        return false;

    if (bot->HasAura(SPELL_GRAVEYARD_GAS_DAMAGE))
        return true;

    Creature* zanzil = FindBoss(bot, NPC_ZANZIL);
    if (!zanzil || !zanzil->IsInCombat())
        return false;

    return zanzil->HasAura(SPELL_GRAVEYARD_GAS) || zanzil->FindCurrentSpellBySpellId(SPELL_GRAVEYARD_GAS);
}

GameObject* GetToxicTormentCauldron(Player* bot)
{
    if (!bot->IsAlive() || bot->HasAura(SPELL_TOXIC_TORMENT) || !IsGraveyardGasActive(bot))
        return nullptr;

    std::list<GameObject*> cauldrons;
    bot->GetGameObjectListWithEntryInGrid(cauldrons, GO_TOXIC_TORMENT_CAULDRON, TOXIC_TORMENT_SCAN_RANGE);
    GameObject* best = nullptr;
    float bestDist = TOXIC_TORMENT_SCAN_RANGE + 1.0f;
    for (GameObject* cauldron : cauldrons)
    {
        if (!cauldron->isSpawned())
            continue;

        float const dist = bot->GetExactDist(cauldron);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = cauldron;
        }
    }

    return best;
}

bool GetCauldronPlan(Player* bot, CauldronPlan& plan)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat() || !bot->IsAlive() || !IsCauldronCandidate(bot))
        return false;

    // The nearest Berserker that fights the group and is not frozen yet.
    Creature* berserker = nullptr;
    float bestDist = CAULDRON_BERSERKER_RANGE + 1.0f;
    for (Creature* creature : LivingInGrid(bot, NPC_ZANZILI_BERSERKER, CAULDRON_BERSERKER_RANGE))
    {
        if (!creature->IsInCombat() || creature->HasAura(SPELL_FROSTBURN_FORMULA) ||
            creature->HasAura(SPELL_FROSTBURN_HYPOTHERMIA))
            continue;

        float const dist = bot->GetExactDist2d(creature);
        if (dist < bestDist)
        {
            bestDist = dist;
            berserker = creature;
        }
    }

    if (!berserker || ElectCauldronBot(bot, berserker) != bot)
        return false;

    plan.berserker = berserker;
    plan.armed = bot->HasAura(SPELL_FROSTBURN_AURA);
    plan.cauldron = nullptr;
    if (plan.armed)
        return true;

    std::list<GameObject*> cauldrons;
    bot->GetGameObjectListWithEntryInGrid(cauldrons, GO_FROSTBURN_CAULDRON, CAULDRON_SCAN_RANGE);
    float cauldronDist = CAULDRON_SCAN_RANGE + 1.0f;
    for (GameObject* cauldron : cauldrons)
    {
        if (!cauldron->isSpawned())
            continue;

        float const dist = bot->GetExactDist(cauldron);
        if (dist < cauldronDist)
        {
            cauldronDist = dist;
            plan.cauldron = cauldron;
        }
    }

    return plan.cauldron != nullptr;
}

// Jin'do chain phase: a non-tank bot whose current target is a Gurubashi Spirit drops it (the spirit has to live long
// enough to Body Slam the chains); see GetFocusTarget.
bool ShouldSpareSpirit(Player* bot, Unit* currentTarget)
{
    if (!currentTarget || currentTarget->GetEntry() != NPC_GURUBASHI_SPIRIT || PlayerBotSpec::IsTank(bot))
        return false;

    return bot->GetMapId() == MAP_ZUL_GURUB && GetShieldedChain(bot) != nullptr;
}

// #679 w106: an interrupt only lands on a cast that Spell::EffectInterruptCast accepts (SpellEffects.cpp:3866-3867): CASTING, or
// PREPARING with a cast time. Whispers of Hethiss is an instant channel; between DoCast (PREPARING, cast time 0) and the caster's next
// _UpdateSpells (CASTING, aura on) FindCurrentSpellBySpellId already returns it, and w106 burned 4 of 11 interrupts there (Adald
// counterspell + Kaellio pummel right before channel 4, which then ran 17 ticks).
bool IsInterruptibleNow(Unit* caster, uint32 spellId)
{
    Spell const* spell = caster->FindCurrentSpellBySpellId(spellId);
    if (!spell)
        return false;

    return spell->getState() == SPELL_STATE_CASTING || (spell->getState() == SPELL_STATE_PREPARING && spell->GetCastTime() > 0);
}

bool IsVenoxisWhispersFight(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat())
        return false;

    Creature* venoxis = FindBoss(bot, NPC_VENOXIS);
    return venoxis && venoxis->IsInCombat();
}

Unit* GetInterruptTarget(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_GURUB || !bot->IsInCombat())
        return nullptr;

    if (Creature* kilnara = FindBoss(bot, NPC_KILNARA))
    {
        // Tears of Blood is an instant aura on the boss; the fork's script strips it on any interrupt effect.
        if (kilnara->HasAura(SPELL_TEARS_OF_BLOOD))
            return kilnara;

        if (IsInterruptibleNow(kilnara, SPELL_SHADOW_BOLT))
            return kilnara;
    }

    if (Creature* zanzil = FindBoss(bot, NPC_ZANZIL))
    {
        if (IsInterruptibleNow(zanzil, SPELL_VOODOO_BOLT))
            return zanzil;
    }

    // #679: Whispers of Hethiss is a channel (CURRENT_CHANNELED_SPELL); FindCurrentSpellBySpellId looks at every spell slot. Only once
    // the channel is live (w106: see IsInterruptibleNow).
    if (Creature* venoxis = FindBoss(bot, NPC_VENOXIS))
    {
        if (venoxis->IsInCombat() && IsInterruptibleNow(venoxis, SPELL_WHISPERS_OF_HETHISS))
            return venoxis;
    }

    return nullptr;
}

namespace
{
char const* const INTERRUPTS[] = { "kick",         "pummel",         "shield bash",  "wind shear",
                                   "mind freeze",  "counterspell",   "spell lock",   "silencing shot",
                                   "skull bash",   "spear hand strike", "rebuke",     "strangulate",
                                   "counter shot" }; // #679 w106: the 5.4 hunter baseline interrupt
}

bool CanCastDedicatedInterrupt(PlayerbotAI* botAI, Unit* target)
{
    for (char const* name : INTERRUPTS)
    {
        if (botAI->CanCastSpell(std::string(name), target))
            return true;
    }

    return false;
}

bool IsClassInterruptAction(std::string const& name)
{
    // The class strategies' interrupt actions: "<spell>", "<spell> on enemy caster", "<spell> on enemy healer", "rebuke interrupt".
    for (char const* spell : INTERRUPTS)
    {
        std::string const base(spell);
        if (name == base || name.rfind(base + " on enemy", 0) == 0 || name == base + " interrupt")
            return true;
    }

    return false;
}

bool CastDedicatedInterrupt(PlayerbotAI* botAI, Unit* target)
{
    for (char const* name : INTERRUPTS)
    {
        std::string const spell(name);
        if (!botAI->CanCastSpell(spell, target))
            continue;

        if (botAI->CastSpell(spell, target))
            return true;
    }

    return false;
}
}
