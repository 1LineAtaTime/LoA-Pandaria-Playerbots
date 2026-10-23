/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCTRIGGERS_H
#define PLAYERBOTS_OCTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/OC/OCTriggers.h:15
// 5.4.8 spell-id decisions (epic #569 SPELL-ID RULE), all checked against src/server/scripts/Northrend/Nexus/Oculus/:
//  - Drake / vehicle ability ids (Amber 49840 49836 49838 49592, Emerald 50328 50341 50344, Ruby 50232 50240
//    50241 50253) are the same ids as AC and are exactly the spell1-3 of the MoP creature_template rows
//    27755 / 27692 / 27756 (oculus.cpp:43-71 lists the same ids); Spell.dbc names match.
//  - Essence items 37859 / 37815 / 37860 exist in the MoP item_template with spellid_1 = 49461 / 49345 / 49462
//    (oculus.cpp:24,32-34,41,54,66).
//  - SPELL_ENRAGED_ASSAULT 51170 and SPELL_PLANAR_SHIFT 51162 are what the fork's Eregos casts
//    (boss_eregos.cpp:48,50,185,218), single id in every difficulty.
//  - SPELL_TIME_BOMB: the fork's Urom casts only 51121 (boss_urom.cpp:40,334); SPELL_TIME_BOMB_2 = 59376 is
//    declared (boss_urom.cpp:41) but never cast. Heroic keeps the same id with a difficulty SpellInfo
//    (src/server/game/Spells/SpellMgr.cpp:471-476), so the macro is the normal id, no DUNGEON_MODE.
//  - SPELL_EMPOWERED_ARCANE_EXPLOSION: the fork script itself switches in heroic
//    (boss_urom.cpp:292 DUNGEON_MODE(51110, 59377); the cast check at :284 uses the normal id), so
//    DUNGEON_MODE is kept, as in AC OCTriggers.h:64.
enum OculusIDs
{
    // Drakos the Interrogator
    NPC_UNSTABLE_SPHERE             = 28166,
    SPELL_UNSTABLE_SPHERE_PASSIVE   = 50756,
    SPELL_UNSTABLE_SPHERE_PULSE     = 50757,
    SPELL_UNSTABLE_SPHERE_TIMER     = 50758,

    // Drakes
    NPC_AMBER_DRAKE                 = 27755,
    NPC_EMERALD_DRAKE               = 27692,
    NPC_RUBY_DRAKE                  = 27756,
    ITEM_AMBER_ESSENCE              = 37859,
    ITEM_EMERALD_ESSENCE            = 37815,
    ITEM_RUBY_ESSENCE               = 37860,
    SPELL_AMBER_ESSENCE             = 49461,
    SPELL_EMERALD_ESSENCE           = 49345,
    SPELL_RUBY_ESSENCE              = 49462,
    // Abilities:
    // Amber
    SPELL_SHOCK_LANCE               = 49840,
    SPELL_SHOCK_CHARGE              = 49836,
    SPELL_STOP_TIME                 = 49838,
    SPELL_TEMPORAL_RIFT             = 49592,
    // Emerald
    SPELL_LEECHING_POISON           = 50328,
    SPELL_TOUCH_THE_NIGHTMARE       = 50341,
    SPELL_DREAM_FUNNEL              = 50344,
    // Ruby
    SPELL_SEARING_WRATH             = 50232,
    SPELL_EVASIVE_MANEUVERS         = 50240,
    SPELL_EVASIVE_CHARGES           = 50241,
    SPELL_MARTYR                    = 50253,

    // Varos Cloudstrider
    NPC_CENTRIFUGE_CORE             = 28183,

    // Mage-Lord Urom
    NPC_MAGE_LORD_UROM              = 27655,
    SPELL_TIME_BOMB_N               = 51121,
    SPELL_TIME_BOMB_H               = 59376,
    SPELL_EMPOWERED_ARCANE_EXPLOSION_N = 51110,
    SPELL_EMPOWERED_ARCANE_EXPLOSION_H = 59377,

    // Ley-Guardian Eregos
    SPELL_ENRAGED_ASSAULT           = 51170,
    SPELL_PLANAR_SHIFT              = 51162,
};

// AC Ai/Dungeon/OC/OCTriggers.h:64
#define SPELL_EMPOWERED_ARCANE_EXPLOSION        DUNGEON_MODE(bot, SPELL_EMPOWERED_ARCANE_EXPLOSION_N, SPELL_EMPOWERED_ARCANE_EXPLOSION_H)
// AC Ai/Dungeon/OC/OCTriggers.h:65 is DUNGEON_MODE(bot, SPELL_TIME_BOMB_N, SPELL_TIME_BOMB_H); see the note above.
#define SPELL_TIME_BOMB                         SPELL_TIME_BOMB_N

// AC Ai/Dungeon/OC/OCTriggers.h:67-69
const std::vector<uint32> DRAKE_ITEMS = {ITEM_AMBER_ESSENCE, ITEM_EMERALD_ESSENCE, ITEM_RUBY_ESSENCE};
const std::vector<uint32> DRAKE_SPELLS = {SPELL_AMBER_ESSENCE, SPELL_EMERALD_ESSENCE, SPELL_RUBY_ESSENCE};
const uint32 OCULUS_MAP_ID = 578;

// const float uromCoords[4][4] =
// {
//     {1177.47f, 937.722f, 527.405f, 2.21657f},
//     {968.66f, 1042.53f, 527.32f, 0.077f},
//     {1164.02f, 1170.85f, 527.321f, 3.66f},
//     {1118.31f, 1080.377f, 508.361f, 4.25f}      // Inner ring, actual boss fight
// };

// AC Ai/Dungeon/OC/OCTriggers.h:79
class DrakosUnstableSphereTrigger : public Trigger
{
public:
    DrakosUnstableSphereTrigger(PlayerbotAI* ai) : Trigger(ai, "drakos unstable sphere") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:86
class DrakeMountTrigger : public Trigger
{
public:
    DrakeMountTrigger(PlayerbotAI* ai) : Trigger(ai, "drake mount") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:93
class DrakeDismountTrigger : public Trigger
{
public:
    DrakeDismountTrigger(PlayerbotAI* ai) : Trigger(ai, "drake dismount") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:100
class GroupFlyingTrigger : public Trigger
{
public:
    GroupFlyingTrigger(PlayerbotAI* ai) : Trigger(ai, "drake fly") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:107
class DrakeCombatTrigger : public Trigger
{
public:
    DrakeCombatTrigger(PlayerbotAI* ai) : Trigger(ai, "drake combat") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:114
class VarosCloudstriderTrigger : public Trigger
{
public:
    VarosCloudstriderTrigger(PlayerbotAI* ai) : Trigger(ai, "varos cloudstrider") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:121
class UromArcaneExplosionTrigger : public Trigger
{
public:
    UromArcaneExplosionTrigger(PlayerbotAI* ai) : Trigger(ai, "urom arcane explosion") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/OC/OCTriggers.h:128
class UromTimeBombTrigger : public Trigger
{
public:
    UromTimeBombTrigger(PlayerbotAI* ai) : Trigger(ai, "urom time bomb") {}
    bool IsActive() override;
};

#endif
