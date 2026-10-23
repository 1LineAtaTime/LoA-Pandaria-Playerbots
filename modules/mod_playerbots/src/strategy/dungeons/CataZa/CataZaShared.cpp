/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) shared helpers, MoP-only lane L7 (#669). See CataZaShared.h for sources.

#include "CataZaShared.h"
#include "AttackersValue.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
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

namespace CataZulAman
{
namespace
{
constexpr float STORM_SCAN_RANGE = 80.0f;
constexpr float INTERRUPT_SCAN_RANGE = 45.0f;

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

// Valid, attackable, in line of sight: the same gate AC dungeon focus code uses (MgTShared.cpp GetActiveFelCrystal).
Unit* AttackableByEntry(Player* bot, uint32 entry, float range)
{
    Creature* creature = bot->FindNearestCreature(entry, range, true);
    if (!creature || !AttackersValue::IsValidTarget(creature, bot))
        return nullptr;

    return creature;
}

// #669 w68: the Kidnapper to kill is the one a group member rides (Plucked 97318 = control vehicle aura 236, the player's
// vehicle base is the eagle), not the nearest one. Kidnappers are TEMPSUMMON_DEAD_DESPAWN and stay after their passenger
// died: the rogue spent 60 s (431 "za focus target" FAILED = already the target) on the empty 52638:274 hovering 15 yd
// over his head while the healer, plucked by 52638:283, died in its seat and the group wiped.
Unit* GetCarryingKidnapper(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* best = nullptr;
    float bestDist = 0.0f;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        Unit* base = member->GetVehicleBase();
        if (!base || base->GetEntry() != NPC_AMANI_KIDNAPPER || !base->IsAlive() ||
            !AttackersValue::IsValidTarget(base, bot))
            continue;

        float const dist = bot->GetExactDist(base);
        if (!best || dist < bestDist)
        {
            best = base;
            bestDist = dist;
        }
    }

    return best;
}

// A group member the Static Disruption spread would move itself: a bot, not tank or melee, free to walk.
bool MovesForSpread(Player* member)
{
    return GET_PLAYERBOT_AI(member) && member->IsInCombat() && !PlayerBotSpec::IsTank(member) &&
           !PlayerBotSpec::IsMelee(member) && !member->GetVehicle() && !member->HasAura(SPELL_ELECTRICAL_STORM);
}

uint8 CastPriority(Unit* caster)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 prio = 0;
        switch (spell->m_spellInfo->Id)
        {
            case SPELL_ALYSON_FLASH_HEAL:
            case SPELL_PR_HEAL:
            case SPELL_PR_FLASH_HEAL:
            case SPELL_PA_HOLY_LIGHT:
            case SPELL_SH_HEALING_WAVE:
                prio = 1;
                break;
            case SPELL_SH_CHAIN_LIGHTNING:
            case SPELL_VENOM_SPIT:
            case SPELL_MG_FROSTBOLT:
            case SPELL_MG_FIREBALL:
                prio = 2;
                break;
            default:
                break;
        }

        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}
}

float HazardSlack(Hazard const& hazard, float x, float y)
{
    float const dx = x - hazard.x;
    float const dy = y - hazard.y;
    float const dist = std::hypot(dx, dy);

    if (hazard.shape == HazardShape::Circle)
        return dist - hazard.radius;

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

void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN)
        return;

    switch (kind)
    {
        case HazardKind::FireBombs:
        {
            // Jan'alai spawns 40 Fire Bombs over the platform, arms them and detonates them ~6 s later
            // (boss_janalai.cpp:186-196, 266-298); each blast is spell 42630, 4 yd.
            if (!FindBoss(bot, NPC_JANALAI))
                return;

            std::list<Creature*> bombs;
            bot->GetCreatureListWithEntryInGrid(bombs, NPC_FIRE_BOMB, HAZARD_SCAN_RANGE);
            float const radius = SpellRadius(SPELL_FIRE_BOMB_DAMAGE, 4.0f);
            for (Creature* bomb : bombs)
            {
                if (bomb->IsAlive())
                    out.push_back(Circle(bomb->GetPositionX(), bomb->GetPositionY(), radius));
            }

            return;
        }
        case HazardKind::FlameBreath:
        {
            // Cast (1 s) and then an aura that lays a 4 yd fire line in front of the caster every 100 ms
            // (boss_janalai.cpp:419-442 spell_janalai_flame_breath; Daakara reuses 43140 as dragonhawk).
            for (uint32 entry : { NPC_JANALAI, NPC_DAAKARA })
            {
                Creature* boss = FindBoss(bot, entry);
                if (!boss)
                    continue;

                Spell* cast = boss->HasUnitState(UNIT_STATE_CASTING) ? boss->FindCurrentSpellBySpellId(SPELL_FLAME_BREATH)
                                                                      : nullptr;
                if (!cast && !boss->HasAura(SPELL_FLAME_BREATH))
                    continue;

                float angle = boss->GetOrientation();
                if (cast)
                {
                    if (Unit* aimed = ObjectAccessor::GetUnit(*boss, cast->m_targets.GetUnitTargetGUID()))
                        angle = boss->GetAngle(aimed);
                }

                out.push_back(Cone(boss->GetPositionX(), boss->GetPositionY(), FLAME_BREATH_LENGTH, angle,
                                   FLAME_BREATH_HALF_ANGLE));
            }

            float const radius = SpellRadius(SPELL_FLAME_BREATH_LINE, 4.0f);
            for (Position const& pos :
                 EncounterHelpers::GetDynamicObjectPositions(bot, HAZARD_SCAN_RANGE, SPELL_FLAME_BREATH_LINE))
            {
                out.push_back(Circle(pos.GetPositionX(), pos.GetPositionY(), radius));
            }

            return;
        }
        case HazardKind::PillarOfFlame:
        {
            // Daakara dragonhawk: Column of Fire 24187, 9-11k fire per second within ~2 yd for 30 s (boss_daakara.cpp:162-166).
            std::list<Creature*> pillars;
            bot->GetCreatureListWithEntryInGrid(pillars, NPC_PILLAR_OF_FLAME, HAZARD_SCAN_RANGE);
            for (Creature* pillar : pillars)
            {
                if (pillar->IsAlive())
                    out.push_back(Circle(pillar->GetPositionX(), pillar->GetPositionY(), PILLAR_RADIUS));
            }

            return;
        }
        case HazardKind::DaakaraWhirlwind:
        {
            // Phase 1: whirlwind aura 17207 on the boss (boss_daakara.cpp:254-257). The tank cannot step out of it.
            if (PlayerBotSpec::IsTank(bot))
                return;

            Creature* boss = FindBoss(bot, NPC_DAAKARA);
            if (!boss || !boss->HasAura(SPELL_WHIRLWIND))
                return;

            out.push_back(Circle(boss->GetPositionX(), boss->GetPositionY(), WHIRLWIND_RADIUS));
            return;
        }
        case HazardKind::MalacrassGround:
        {
            // Siphon Soul gives Malacrass a random class kit (boss_hexlord.cpp:108-166): Warlock Rain of Fire and
            // Paladin Consecration are persistent ground areas (DynamicObjects).
            if (!FindBoss(bot, NPC_MALACRASS))
                return;

            float const rain = SpellRadius(SPELL_RAIN_OF_FIRE, 5.0f);
            for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, HAZARD_SCAN_RANGE, SPELL_RAIN_OF_FIRE))
                out.push_back(Circle(pos.GetPositionX(), pos.GetPositionY(), rain));

            float const consecration = SpellRadius(SPELL_CONSECRATION, 10.0f);
            for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, HAZARD_SCAN_RANGE, SPELL_CONSECRATION))
                out.push_back(Circle(pos.GetPositionX(), pos.GetPositionY(), consecration));

            // #678 Cata kit: Death Knight Death and Decay 61603, Rogue Smoke Bomb 97644 (both DynamicObjects).
            for (uint32 spellId : { SPELL_DEATH_AND_DECAY, SPELL_SMOKE_BOMB_AREA })
            {
                float const radius = SpellRadius(spellId, 5.0f);
                for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, HAZARD_SCAN_RANGE, spellId))
                    out.push_back(Circle(pos.GetPositionX(), pos.GetPositionY(), radius));
            }

            return;
        }
        case HazardKind::FeatherCyclone:
        {
            // #678 Daakara eagle (Wowpedia "Daakara": "Merely moving away from where you were standing when the whirlwinds
            // begin moving will allow you to avoid them"): every cyclone, and for a moving one its straight path to the end.
            if (!FindBoss(bot, NPC_DAAKARA))
                return;

            std::list<Creature*> cyclones;
            bot->GetCreatureListWithEntryInGrid(cyclones, NPC_FEATHER_VORTEX, HAZARD_SCAN_RANGE);
            for (Creature* cyclone : cyclones)
            {
                if (!cyclone->IsAlive())
                    continue;

                float const cx = cyclone->GetPositionX();
                float const cy = cyclone->GetPositionY();
                out.push_back(Circle(cx, cy, CYCLONE_RADIUS));

                if (!cyclone->movespline || cyclone->movespline->Finalized())
                    continue;

                auto const dest = cyclone->movespline->FinalDestination();
                float const dx = dest.x - cx;
                float const dy = dest.y - cy;
                float const len = std::hypot(dx, dy);
                if (len < 0.5f || len > HAZARD_SCAN_RANGE)
                    continue;

                for (float d = CYCLONE_PATH_STEP; d < len; d += CYCLONE_PATH_STEP)
                    out.push_back(Circle(cx + dx / len * d, cy + dy / len * d, CYCLONE_RADIUS));
                out.push_back(Circle(dest.x, dest.y, CYCLONE_RADIUS));
            }

            return;
        }
    }
}

bool FindEscapeSpot(Player* bot, std::vector<Hazard> const& hazards, float margin, float& outX, float& outY,
                    float& outZ, Unit const* anchor, float anchorRange, float maxFromHazards)
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

                if (anchor && (anchor->GetExactDist2d(candidate.x, candidate.y) > anchorRange ||
                               !(std::fabs(z - anchor->GetPositionZ()) <= 3.0f)))
                    continue;

                if (maxFromHazards > 0.0f &&
                    std::any_of(hazards.begin(), hazards.end(), [&](Hazard const& hazard)
                                { return std::hypot(candidate.x - hazard.x, candidate.y - hazard.y) > maxFromHazards; }))
                    continue;

                if (!bot->IsWithinLOS(candidate.x, candidate.y, z + 2.0f))
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

bool GetStormEye(Player* bot, float& centreX, float& centreY, float& centreZ, float& radius)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat())
        return false;

    // The victim cannot move; inside the eye there is nothing to do. A Plucked bot rides the Kidnapper and cannot move
    // either (w68: the plucked mage logged 8 "za join storm eye" FAILED under the second storm).
    if (bot->HasAura(SPELL_ELECTRICAL_STORM) || bot->HasAura(SPELL_ELECTRICAL_SAFE) || bot->GetVehicle())
        return false;

    float bestDist = 0.0f;
    bool found = false;
    for (Position const& eye : EncounterHelpers::GetDynamicObjectPositions(bot, STORM_SCAN_RANGE, SPELL_ELECTRICAL_SAFE))
    {
        float const dist = bot->GetExactDist2d(eye);
        if (!found || dist < bestDist)
        {
            found = true;
            bestDist = dist;
            centreX = eye.GetPositionX();
            centreY = eye.GetPositionY();
            centreZ = eye.GetPositionZ();
        }
    }

    if (!found)
        return false;

    radius = SpellRadius(SPELL_ELECTRICAL_SAFE, 10.0f);
    return true;
}

bool IsStormActive(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat())
        return false;

    return !EncounterHelpers::GetDynamicObjectPositions(bot, STORM_SCAN_RANGE, SPELL_ELECTRICAL_SAFE).empty();
}

bool NeedsSpread(Player* bot, Player*& nearest)
{
    nearest = nullptr;
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat())
        return false;

    // Melee stack on the boss and the tank holds it; ranged and healers carry the spread.
    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsMelee(bot))
        return false;

    if (!FindBoss(bot, NPC_AKILZON))
        return false;

    // #669 w67: during an Electrical Storm the eye is the only safe place, but the spread fired inside it (everybody is
    // stacked there) and walked the bot back out: "T:za electrical storm" and "T:za static spread" on the same ticks.
    // A lifted (43648) or Plucked (Kidnapper vehicle) bot cannot move at all.
    if (IsStormActive(bot) || bot->HasAura(SPELL_ELECTRICAL_STORM) || bot->GetVehicle())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float best = SPREAD_MIN_DISTANCE;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        // #669 w69: two ranged spreading off each other at the same time chased each other round the platform (healer
        // 41 "za static spread" FAILED = still walking / 12 OK in the first 68 s; 56 Heal + 56 Greater Heal "why=moving"),
        // so the healer barely cast and the mage died at 34 s. Of two spreading bots only the higher GUID moves.
        if (bot->GetGUID() < member->GetGUID() && MovesForSpread(member))
            continue;

        float const dist = bot->GetDistance2d(member);
        if (dist < best)
        {
            best = dist;
            nearest = member;
        }
    }

    return nearest != nullptr;
}

void CollectSpreadHazards(Player* bot, std::vector<Hazard>& out)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        out.push_back(Circle(member->GetPositionX(), member->GetPositionY(), SPREAD_MIN_DISTANCE));
    }
}

Unit* GetSurgeAnchor(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat() || PlayerBotSpec::IsTank(bot) ||
        !bot->HasAura(SPELL_SURGE))
        return nullptr;

    for (uint32 entry : { NPC_NALORAKK, NPC_DAAKARA })
    {
        Creature* boss = FindBoss(bot, entry);
        if (boss && bot->GetExactDist2d(boss) > SURGE_HOLD_DISTANCE)
            return boss;
    }

    return nullptr;
}

Unit* GetWaterTotemToLeave(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat() || !PlayerBotSpec::IsTank(bot))
        return nullptr;

    Creature* boss = FindBoss(bot, NPC_HALAZZI);
    if (!boss || boss->GetVictim() != bot)
        return nullptr;

    Creature* totem = bot->FindNearestCreature(NPC_WATER_TOTEM, 40.0f, true);
    if (!totem || boss->GetExactDist2d(totem) > WATER_TOTEM_RADIUS)
        return nullptr;

    return totem;
}

Unit* GetFocusTarget(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat())
        return nullptr;

    // #669 w67: Akil'zon's tank holds the boss (Call Lightning hits his current target, Wowpedia "Akil'zon (tactics)").
    // Its tank-assist picker chased the Soaring Eagles 24858 instead (flying adds, SetCanFly, out of melee reach: tank
    // 109 probe ticks on eagles vs 58 on the boss, 749 "melee - FAILED"), so the boss ran loose on the group.
    if (PlayerBotSpec::IsTank(bot))
    {
        Creature* akilzon = FindBoss(bot, NPC_AKILZON);
        if (akilzon && akilzon->IsInCombat() && AttackersValue::IsValidTarget(akilzon, bot))
            return akilzon;

        return nullptr;
    }

    // Same role gate as MgTTriggers.cpp MgTFocusTargetTrigger: DPS only, never the tank or a healer.
    if (!PlayerBotSpec::IsDps(bot))
        return nullptr;

    // Akil'zon: the Amani Kidnapper carries a Plucked player (4% max health per second); killing it frees them.
    if (FindBoss(bot, NPC_AKILZON))
    {
        if (Unit* kidnapper = GetCarryingKidnapper(bot))
            return kidnapper;
    }

    // Jan'alai: kill the Hatchers before they hatch the eggs (Wowpedia: hatchlings are the dangerous part).
    if (FindBoss(bot, NPC_JANALAI))
    {
        for (uint32 entry : { NPC_HATCHER_1, NPC_HATCHER_2 })
        {
            if (Unit* hatcher = AttackableByEntry(bot, entry, 90.0f))
                return hatcher;
        }
    }

    // Halazzi: the Corrupted Lightning Totem ramps up fast, Wowpedia "must be killed quickly" (10k health).
    // #678 Daakara eagle summons the same totem 24224 (97930): Wowpedia "This totem has low health".
    if (FindBoss(bot, NPC_HALAZZI) || FindBoss(bot, NPC_DAAKARA))
    {
        if (Unit* totem = AttackableByEntry(bot, NPC_LIGHTNING_TOTEM, 50.0f))
            return totem;
    }

    // Malacrass: two random servants; healer first, then the hardest casters. Burn the boss at the end.
    if (Creature* malacrass = FindBoss(bot, NPC_MALACRASS))
    {
        // #678 Death Knight kit: each Blood Worm that reaches him heals 3% (Wowpedia), so worms come first at any boss health.
        if (Unit* worm = AttackableByEntry(bot, NPC_BLOOD_WORM, 60.0f))
            return worm;

        if (malacrass->GetHealthPct() > EncounterHelpers::BOSS_BURN_HEALTH_PCT)
        {
            for (uint32 entry : { NPC_ALYSON_ANTILE, NPC_SLITHER, NPC_DARKHEART, NPC_GAZAKROTH })
            {
                if (Unit* servant = AttackableByEntry(bot, entry, 80.0f))
                    return servant;
            }
        }
    }

    return nullptr;
}

Unit* GetInterruptTarget(Player* bot)
{
    if (bot->GetMapId() != MAP_ZUL_AMAN || !bot->IsInCombat() || !FindBoss(bot, NPC_MALACRASS))
        return nullptr;

    Unit* best = nullptr;
    uint8 bestPrio = 0;
    for (uint32 entry : { NPC_MALACRASS, NPC_ALYSON_ANTILE, NPC_SLITHER, NPC_GAZAKROTH, NPC_DARKHEART })
    {
        Creature* caster = bot->FindNearestCreature(entry, INTERRUPT_SCAN_RANGE, true);
        if (!caster || !caster->IsNonMeleeSpellCasted(true))
            continue;

        uint8 const prio = CastPriority(caster);
        if (prio && (!best || prio < bestPrio))
        {
            best = caster;
            bestPrio = prio;
        }
    }

    return best;
}

bool CastDedicatedInterrupt(PlayerbotAI* botAI, Unit* target)
{
    static char const* const INTERRUPTS[] = { "kick",         "pummel",         "shield bash",  "wind shear",
                                              "mind freeze",  "counterspell",   "spell lock",   "silencing shot",
                                              "skull bash",   "spear hand strike", "rebuke",     "strangulate" };

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
