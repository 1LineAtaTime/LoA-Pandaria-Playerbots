/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSTRIGGERS_H
#define PLAYERBOTS_FOSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/FoS/FoSTriggers.h:15
enum ForgeOfSoulsBronjahmIDs
{
    // Bronjahm
    // 5.4.8 SPELL-ID RULE (epic #569): AC has no DUNGEON_MODE on any FoS id and none is needed. The fork's boss
    // script casts these same ids in every difficulty
    // (src/server/scripts/Northrend/FrozenHalls/ForgeOfSouls/boss_bronjahm.cpp:35,41,174,180-181) and a heroic
    // cast keeps the same id with a difficulty SpellInfo (src/server/game/Spells/SpellMgr.cpp:471-476).
    // NPC 36535 is the same entry and name ("Corrupted Soul Fragment") in MoP and AC creature_template.
    NPC_CORRUPTED_SOUL_FRAGMENT = 36535,

    // boss_bronjahm.cpp:35,174; SpellEffect.dbc: effect 0 applies aura 4 (dummy) to the target, so HasAura works
    SPELL_CORRUPT_SOUL          = 68839,
    SPELL_SOULSTORM_VISUAL      = 68870,  // boss_bronjahm.cpp:41,180
    // The fork never has 68904 as the boss's current spell: it only fires 68904-68907 as triggered casts from the
    // soulstorm channel periodic tick (boss_bronjahm.cpp:334-335). Kept as in AC.
    SPELL_SOULSTORM_VISUAL2     = 68904,
    SPELL_SOULSTORM             = 68872,  // boss_bronjahm.cpp:39,181 (unused by AC's strategy logic)

    // Devourer of Souls
    // Unused by AC's strategy logic. The fork's boss script uses 68875 (beam) / 68873 (spell) for wailing souls
    // (boss_devourer_of_souls.cpp:57-58), not 68899.
    SPELL_WAILING_SOULS         = 68899,
};

// AC Ai/Dungeon/FoS/FoSTriggers.h:29
class MoveFromBronjahmTrigger : public Trigger
{
public:
    MoveFromBronjahmTrigger(PlayerbotAI* ai) : Trigger(ai, "move from bronjahm") {}

    bool IsActive() override;
};

// AC Ai/Dungeon/FoS/FoSTriggers.h:37
class SwitchToSoulFragment : public Trigger
{
public:
    SwitchToSoulFragment(PlayerbotAI* ai) : Trigger(ai, "switch to soul fragment") {}

    bool IsActive() override;
};

// AC Ai/Dungeon/FoS/FoSTriggers.h:45
class BronjahmPositionTrigger : public Trigger
{
public:
    BronjahmPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "bronjahm position") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/FoS/FoSTriggers.h:52
class DevourerOfSoulsTrigger : public Trigger
{
public:
    DevourerOfSoulsTrigger(PlayerbotAI* ai) : Trigger(ai, "devourer of souls") {}
    bool IsActive() override;
};

#endif
