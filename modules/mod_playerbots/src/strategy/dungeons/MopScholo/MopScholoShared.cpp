/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#include "MopScholoShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Group.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <list>

namespace MopScholomance
{
namespace
{
// Same stop-a-cast test as the fork's MgT strategy (MgTShared.cpp:507): a silence-type interrupt, an explicit
// interrupt-cast effect, or a hard stun/silence aura.
bool StopsACast(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return false;

    if ((info->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
        info->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
        return true;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; ++i)
    {
        if (info->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
            return true;

        if (info->Effects[i].Effect != SPELL_EFFECT_APPLY_AURA)
            continue;

        if (info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE ||
            info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_STUN)
            return true;
    }

    return false;
}

// Dedicated interrupts first, hard control last (MgTShared.cpp:545-566, plus the MoP-only names: Monk
// "spear hand strike", Paladin "rebuke", Druid "skull bash", Hunter "counter shot").
char const* const INTERRUPT_SPELLS[] = {
    "kick", "pummel", "shield bash", "wind shear", "mind freeze", "counterspell", "spell lock",
    "spear hand strike", "rebuke", "skull bash", "counter shot", "silencing shot",
    "shockwave", "shadowfury", "hammer of justice", "kidney shot", "silence", "bash", "strangulate",
    "arcane torrent"
};

// The Phylactery once Chillheart's body has fallen: Reset() flags it NOT_SELECTABLE | NON_ATTACKABLE and
// ACTION_ACTIVATE (boss_instructor_chillheart.cpp:351-353) removes both. The tome adds around it take no damage
// (npc_magical_tomesAI::DamageTaken, :559-562), so attacking anything else is wasted.
// #659 w52: only inside the Reliquary. Until it dies the Phylactery stays attackable and Chillheart IN_PROGRESS, and at
// 100 yd (BOSS_SCAN_RANGE) it was found from Jandice's hall (passage mouth 263,56.5 -> 200.6,21.2 = 71.7 yd): every
// non-healer's focus target was the unreachable Phylactery and MopScholoFocusMultiplier zeroed their assist actions, so
// nobody hit Jandice (w52: focus target 1616 FAILED, 1318 no-target probe ticks, wipe at 67%). Reliquary points are
// <= 46 yd from it (main floor corners 38 yd, side lobes 45.5 yd, landing 29.3 yd); Jandice's hall is >= 70 yd.
constexpr float PHYLACTERY_RANGE = 50.0f;

Unit* FindPhylactery(Player* bot)
{
    Creature* phylactery = bot->FindNearestCreature(NPC_PHYLACTERY, PHYLACTERY_RANGE, true);
    if (!phylactery || phylactery->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        phylactery->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) || !bot->IsValidAttackTarget(phylactery))
        return nullptr;

    return phylactery;
}

// Jandice's illusion phases: 14 clones (entry 59220, 42900 hp) hide the real boss (entry 59184, 3432 hp) and the
// phase only ends when the real one drops (boss_jandice_barov.cpp:208-221); killing a clone only costs the group
// (Flash Bang on heroic, the Attention to Detail worldstate). The entries differ, so the real one is trivial to find.
Unit* FindRealJandice(Player* bot)
{
    if (!FindBoss(bot, NPC_JANDICE_ILLUSION))
        return nullptr;

    Creature* real = FindBoss(bot, NPC_JANDICE_BAROV);
    if (!real || !bot->IsValidAttackTarget(real))
        return nullptr;

    return real;
}

// Gandling channels Rise (aura 113143, -50% damage taken, boss_grandmaster_gandling.cpp:185-190 casts it) and the
// Failed Students (59078) swarm the room; Wowpedia: "Focus on them instead of Gandling".
// #659 (w44 gandling leg): 25 students of 1.21 M hp each (heroic health_mod 2.0); with a nearest-student pick each damage
// dealer switched targets 7-9 times in 25 s, every one a different student, and no student fell below 60 %. The bot now
// keeps its current student and otherwise joins the most damaged one, so the damage dealers converge and kill them.
Unit* FindFailedStudent(Player* bot)
{
    Creature* gandling = FindBoss(bot, NPC_GRANDMASTER_GANDLING);
    if (!gandling || !(gandling->HasAura(SPELL_RISE) || gandling->FindCurrentSpellBySpellId(SPELL_RISE)))
        return nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
    {
        Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (current && current->GetEntry() == NPC_FAILED_STUDENT && current->IsAlive() &&
            bot->IsValidAttackTarget(current) && bot->GetExactDist2d(current) <= TARGET_SCAN_RANGE)
            return current;
    }

    std::list<Creature*> students;
    bot->GetCreatureListWithEntryInGrid(students, NPC_FAILED_STUDENT, TARGET_SCAN_RANGE);

    Creature* best = nullptr;
    float bestPct = FLT_MAX;
    float bestDist = FLT_MAX;
    for (Creature* student : students)
    {
        if (!student || !student->IsAlive() || !bot->IsValidAttackTarget(student))
            continue;

        float const pct = student->GetHealthPct();
        float const dist = bot->GetExactDist2d(student);
        if (pct < bestPct - 0.5f || (pct <= bestPct + 0.5f && dist < bestDist))
        {
            best = student;
            bestPct = pct;
            bestDist = dist;
        }
    }

    return best;
}

// #659: Chillheart's body while her first lesson (and so the Ice Wave) is on; nullptr otherwise.
Unit* FindChillheartBody(Player* bot)
{
    float shortfall = 0.0f;
    if (IceWallSafeY(bot, shortfall) == 0.0f)
        return nullptr;

    Creature* boss = FindBoss(bot, NPC_INSTRUCTOR_CHILLHEART);
    if (!boss || !bot->IsValidAttackTarget(boss))
        return nullptr;

    return boss;
}

// #659 Lilian Voss stage 2: at 50% her body goes unselectable (EVENT_SOUL_DISMISS, boss_lilian_voss.cpp:346-351) and
// her Soul (58791) is summoned; only the Soul's "death" brings the body back (npc_lilian_soul DamageTaken -> EVENT_RESSURECT
// -> ACTION_LILIAN_RESSURECTED, :403-447), after which the Soul is NOT_SELECTABLE again. Wowpedia "Lilian Voss (tactics)"
// (MediaWiki API, fetched 2026-10-14): "All other players should unload on the spirit". AC "find target" (threat list,
// FindEngagedBoss): the Soul's DoZoneInCombat (:386) puts every player on it, and no far scan (w52 Phylactery trap).
Unit* FindLilianSoul(Player* bot)
{
    Creature* soul = FindEngagedBoss(bot, NPC_LILIAN_SOUL);
    if (!soul || soul->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || soul->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        !bot->IsValidAttackTarget(soul))
        return nullptr;

    return soul;
}

// #659 Rattlegore: each Bone Pile stands ~2 yd from a Soulflame stalker 45979 (MoP world DB spawns, e.g. pile
// 245.0,81.8 / stalker 246.6,80.5). A pile next to a burning one (aura 114007 for 15 s, boss_rattlegore.cpp:231-234)
// would send the bot back into the pool every tick: "scholo leave soulflame" (ACTION_EMERGENCY + 5) moves it out and
// "scholo get bone armor" (ACTION_MOVE + 5) back to the nearest pile. Such a pile is skipped while the stalker burns.
bool PileInSoulflame(Creature* pile)
{
    std::list<Creature*> stalkers;
    pile->GetCreatureListWithEntryInGrid(stalkers, HAZARD_SOULFLAME.entry, 15.0f);
    for (Creature* stalker : stalkers)
    {
        if (!stalker || !stalker->IsAlive() || !stalker->HasAura(HAZARD_SOULFLAME.aura))
            continue;

        // the hazard's clear distance (FindNearestHazard) plus the 2 yd the bot stops short of the pile
        float const clear = HAZARD_SOULFLAME.radius + HAZARD_SOULFLAME.scaleRadius * stalker->GetObjectScale() +
                            HAZARD_SOULFLAME.margin + 2.0f;
        if (pile->GetExactDist2d(stalker) < clear)
            return true;
    }

    return false;
}
}

Creature* FindBoss(Player* bot, uint32 entry)
{
    return bot->FindNearestCreature(entry, BOSS_SCAN_RANGE, true);
}

Creature* FindEngagedBoss(Player* bot, uint32 entry)
{
    for (auto const& [guid, ref] : bot->GetThreatManager().GetThreatenedByMeList())
    {
        Creature* unit = ref ? ref->GetOwner() : nullptr;
        if (unit && unit->GetEntry() == entry && unit->IsAlive())
            return unit;
    }

    return nullptr;
}

float IceWallSafeY(Player* bot, float& shortfall)
{
    shortfall = 0.0f;
    if (bot->GetMapId() != MAP_SCHOLOMANCE)
        return 0.0f;

    // The wall is up from Chillheart's engage until her fake death (DamageTaken flags her NOT_SELECTABLE and destroys
    // the wall, boss_instructor_chillheart.cpp:229-246) or her reset (:95-96).
    Creature* boss = FindBoss(bot, NPC_INSTRUCTOR_CHILLHEART);
    if (!boss || !boss->IsInCombat() || boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 0.0f;

    Creature* stalker = FindBoss(bot, NPC_ICE_WALL_STALKER);
    if (!stalker)
        return 0.0f;

    // The first row stands at the stalker (ACTION_ACTIVATE, :416-420); later rows walk +Y in world space (:460).
    float front = stalker->GetPositionY();
    std::list<Creature*> rows;
    bot->GetCreatureListWithEntryInGrid(rows, NPC_ICE_WALL_ROW, BOSS_SCAN_RANGE);
    for (Creature* row : rows)
        if (row && row->IsAlive() && row->GetPositionY() > front)
            front = row->GetPositionY();

    float const safeY = front + ICE_WALL_REACH + ICE_WALL_MARGIN;
    shortfall = safeY - bot->GetPositionY();
    // #659: once the main floor is closing, a bot still on it is short of the landing (its only way out, MopScholoShared.h).
    if (safeY + 2.0f > RELIQUARY_FLOOR_ESCAPE_Y && bot->GetPositionZ() < RELIQUARY_MAIN_FLOOR_MAX_Z &&
        bot->GetPositionY() < RELIQUARY_MAIN_FLOOR_MAX_Y)
        shortfall = std::max(shortfall, RELIQUARY_LANDING_Y - bot->GetPositionY());
    return safeY;
}

Creature* FindNearestHazard(Player* bot, Hazard const& hazard, float& clearDistance)
{
    clearDistance = hazard.radius + hazard.margin;

    std::list<Creature*> carriers;
    bot->GetCreatureListWithEntryInGrid(carriers, hazard.entry, hazard.radius + hazard.scaleRadius * 4.0f + hazard.margin + 10.0f);

    Creature* nearest = nullptr;
    float nearestPenetration = 0.0f;
    for (Creature* carrier : carriers)
    {
        if (!carrier || !carrier->IsAlive() || (hazard.aura && !carrier->HasAura(hazard.aura)))
            continue;

        float const clear = hazard.radius + hazard.scaleRadius * carrier->GetObjectScale() + hazard.margin;
        float const penetration = clear - bot->GetExactDist(carrier);
        if (penetration > nearestPenetration)
        {
            nearest = carrier;
            nearestPenetration = penetration;
            clearDistance = clear;
        }
    }

    return nearest;
}

Player* FindNearestGroupMember(Player* bot, float within)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearest = nullptr;
    float nearestDist = within;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist < nearestDist)
        {
            nearest = member;
            nearestDist = dist;
        }
    }

    return nearest;
}

uint8 InterruptPriority(uint32 spellId)
{
    // #781 (owner rule 2026-10-12, sources in MopScholoShared.h): no source asks for an interrupt of Wondrous Rapidity
    // (a tank sidestep instead), Bone Spike (Bone Armor counters it) or Incinerate / Immolate, so none is listed.
    switch (spellId)
    {
        case SPELL_BONE_SPIKE:
        case SPELL_WONDROUS_RAPIDITY:
        case SPELL_INCINERATE:
        case SPELL_IMMOLATE:
        default:
            return 0;
    }
}

uint8 CurrentCastPriority(Unit* caster)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 const prio = InterruptPriority(spell->m_spellInfo->Id);
        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}

Unit* FindInterruptTarget(Player* bot)
{
    Unit* best = nullptr;
    uint8 bestPrio = 0;
    for (uint32 entry : { NPC_JANDICE_BAROV, NPC_RATTLEGORE, NPC_GRANDMASTER_GANDLING })
    {
        Creature* caster = FindBoss(bot, entry);
        if (!caster || !caster->IsNonMeleeSpellCasted(true))
            continue;

        uint8 const prio = CurrentCastPriority(caster);
        if (prio && (!bestPrio || prio < bestPrio))
        {
            best = caster;
            bestPrio = prio;
        }
    }

    return best;
}

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target->IsNonMeleeSpellCasted(true))
        return {};

    for (char const* spellName : INTERRUPT_SPELLS)
    {
        std::string const name = spellName;
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (!spellId || !StopsACast(spellId))
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

bool InHarshLessonRoom(Player* bot)
{
    return FindLessonTarget(bot) != nullptr;
}

Unit* FindLessonTarget(Player* bot)
{
    Creature* gandling = bot->FindNearestCreature(NPC_GRANDMASTER_GANDLING, LESSON_BOSS_SCAN, true);
    if (!gandling || !gandling->IsInCombat())
        return nullptr;

    std::list<Creature*> subjects;
    bot->GetCreatureListWithEntryInGrid(subjects, NPC_EXPIRED_TEST_SUBJECT, LESSON_ROOM_RANGE);

    Creature* nearest = nullptr;
    float nearestDist = FLT_MAX;
    for (Creature* subject : subjects)
    {
        if (!subject || !subject->IsAlive() || std::fabs(subject->GetPositionZ() - bot->GetPositionZ()) >= LESSON_ROOM_DZ ||
            !bot->IsValidAttackTarget(subject))
            continue;

        float const dist = bot->GetExactDist2d(subject);
        if (dist < nearestDist)
        {
            nearest = subject;
            nearestDist = dist;
        }
    }

    return nearest;
}

Creature* FindExplosivePainSubject(Player* bot, uint32* hits)
{
    if (hits)
        *hits = 0;

    std::list<Creature*> fresh;
    bot->GetCreatureListWithEntryInGrid(fresh, NPC_FRESH_TEST_SUBJECT, LESSON_ROOM_RANGE);
    std::list<Creature*> expired;
    bot->GetCreatureListWithEntryInGrid(expired, NPC_EXPIRED_TEST_SUBJECT, LESSON_ROOM_RANGE + LESSON_BLAST_RADIUS);

    Creature* best = nullptr;
    uint32 bestHits = 0;
    for (Creature* subject : fresh)
    {
        if (!subject || !subject->IsAlive() || !subject->HasAura(SPELL_EXPLOSIVE_PAIN) ||
            std::fabs(subject->GetPositionZ() - bot->GetPositionZ()) >= LESSON_ROOM_DZ || !bot->IsWithinLOSInMap(subject))
            continue;

        uint32 count = 0;
        for (Creature* enemy : expired)
            if (enemy && enemy->IsAlive() && subject->GetExactDist(enemy) <= LESSON_BLAST_RADIUS)
                ++count;

        if (count > bestHits)
        {
            best = subject;
            bestHits = count;
        }
    }

    if (hits)
        *hits = bestHits;

    return best;
}

namespace
{
// MoP friendly dispels (the offensive "dispel magic" / "purge" only work on enemies).
char const* const FRIENDLY_DISPELS[] = { "purify", "cleanse", "detox", "nature's cure", "purify spirit", "cleanse spirit" };
}

bool HasFriendlyDispel(PlayerbotAI* botAI)
{
    for (char const* spellName : FRIENDLY_DISPELS)
        if (botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get())
            return true;

    return false;
}

bool CastFriendlyDispel(PlayerbotAI* botAI, Unit* target)
{
    for (char const* spellName : FRIENDLY_DISPELS)
    {
        std::string const name = spellName;
        if (!botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get() || !botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return true;
    }

    return false;
}

bool LessonExitNeeded(Player* bot, Creature*& gandling)
{
    gandling = bot->FindNearestCreature(NPC_GRANDMASTER_GANDLING, LESSON_BOSS_SCAN, true);
    if (!gandling || !gandling->IsInCombat() || bot->GetExactDist(gandling) <= LESSON_RETURN_RANGE)
        return false;

    return FindLessonTarget(bot) == nullptr;
}

Unit* FindFocusTarget(Player* bot)
{
    // #806: inside a Harsh Lesson classroom the Expired Test Subjects come first. A healer with a dispel and a loaded
    // Fresh Test Subject dispels instead (MopScholoHarshLessonAction), otherwise it kills its way out like everyone else.
    if (Unit* lesson = FindLessonTarget(bot))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!PlayerBotSpec::IsHeal(bot) || !botAI || !FindExplosivePainSubject(bot) || !HasFriendlyDispel(botAI))
            return lesson;
    }

    if (PlayerBotSpec::IsHeal(bot))
        return nullptr;

    if (Unit* phylactery = FindPhylactery(bot))
        return phylactery;

    if (Unit* jandice = FindRealJandice(bot))
        return jandice;

    if (Unit* soul = FindLilianSoul(bot))
        return soul;

    if (PlayerBotSpec::IsDps(bot) && !PlayerBotSpec::IsTank(bot))
    {
        if (Unit* body = FindChillheartBody(bot))
            return body;

        return FindFailedStudent(bot);
    }

    return nullptr;
}

void IceWallOutrunSpot(Player* bot, float safeY, float& x, float& y, float& z)
{
    float const goal = safeY + 2.0f;
    if (goal <= RELIQUARY_FLOOR_ESCAPE_Y)
    {
        // Still on the main floor: straight ahead (+Y), as before.
        x = bot->GetPositionX();
        y = goal;
        z = bot->GetPositionZ();
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), x, y, z);
        return;
    }

    z = RELIQUARY_UPPER_Z;
    if (goal <= RELIQUARY_LANDING_MAX_Y)
    {
        x = RELIQUARY_LANDING_X;
        y = std::max(goal, RELIQUARY_LANDING_Y);
        return;
    }

    // #659 w51: the corridor is not a way out. Chillheart's engage closes the Iron Gate 211259 at (200.5,57.1)
    // (boss_instructor_chillheart.cpp:147-154,178; it reopens on her reset / death only), so the corridor mouth just
    // inside it is the last reachable spot (Wowpedia "Instructor Chillheart", 2026-10-14: "escape is not an option").
    x = RELIQUARY_CORRIDOR_X;
    y = std::min(goal, RELIQUARY_MOUTH_Y);
}

bool TankSpot(Player* bot, float& x, float& y, float& z)
{
    if (!PlayerBotSpec::IsTank(bot))
        return false;

    if (FindEngagedBoss(bot, NPC_INSTRUCTOR_CHILLHEART))
    {
        float shortfall = 0.0f;
        float const safeY = IceWallSafeY(bot, shortfall);
        if (safeY == 0.0f)
            return false;

        // Only a spot ahead of the safe line (no tug of war with the outrun): the landing until the safe line reaches it
        // (~77 s), the corridor mouth until ~84 s, then the outrun alone (MopScholoShared.h kill-line timings).
        z = RELIQUARY_UPPER_Z;
        if (safeY <= RELIQUARY_LANDING_Y)
        {
            x = RELIQUARY_LANDING_X;
            y = RELIQUARY_LANDING_Y;
            return true;
        }

        if (safeY <= RELIQUARY_MOUTH_Y)
        {
            x = RELIQUARY_CORRIDOR_X;
            y = RELIQUARY_MOUTH_Y;
            return true;
        }

        return false;
    }

    if (Creature* jandice = FindEngagedBoss(bot, NPC_JANDICE_BAROV))
    {
        // Illusion phases: she is hidden (SetVisible(false), boss_jandice_barov.cpp:198) and her clones are up.
        if (!jandice->IsVisible() || FindBoss(bot, NPC_JANDICE_ILLUSION))
            return false;

        x = JANDICE_SPOT_X;
        y = JANDICE_SPOT_Y;
        z = JANDICE_SPOT_Z;
        return true;
    }

    return false;
}

Creature* FindBonePile(Player* bot)
{
    std::list<Creature*> piles;
    bot->GetCreatureListWithEntryInGrid(piles, NPC_BONE_PILE, BONE_PILE_RANGE);

    // Rattlegore hides the piles (SetVisible(false)) outside the fight (boss_rattlegore.cpp:131-138).
    Creature* nearest = nullptr;
    float nearestDist = FLT_MAX;
    for (Creature* pile : piles)
    {
        if (!pile || !pile->IsAlive() || !pile->IsVisible() || PileInSoulflame(pile))
            continue;

        float const dist = bot->GetExactDist(pile);
        if (dist < nearestDist)
        {
            nearest = pile;
            nearestDist = dist;
        }
    }

    return nearest;
}

bool NeedsBoneArmor(Player* bot)
{
    if (bot->HasAura(SPELL_BONE_ARMOR))
        return false;

    Creature* boss = FindBoss(bot, NPC_RATTLEGORE);
    return boss && boss->IsInCombat();
}

bool FixatedBySoul(Player* bot, Creature*& soul)
{
    if (!bot->HasAura(SPELL_FIXATE_ANGER))
        return false;

    soul = FindBoss(bot, NPC_LILIAN_SOUL);
    return soul && bot->GetExactDist2d(soul) <= FIXATE_REACT_RANGE;
}
}

namespace MopScholomance
{
bool JandiceRapidityActive(Player* bot, Creature*& boss)
{
    boss = FindEngagedBoss(bot, NPC_JANDICE_BAROV);
    // the 1.5 s cast and the barrage that follows are the same spell id (FindCurrentSpellBySpellId covers both slots)
    return boss && boss->FindCurrentSpellBySpellId(SPELL_WONDROUS_RAPIDITY);
}
}
