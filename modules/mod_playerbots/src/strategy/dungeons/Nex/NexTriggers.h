/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXTRIGGERS_H
#define PLAYERBOTS_NEXTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/Nex/NexTriggers.h:15 enum NexusIDs
// 5.4.8 SPELL-ID RULE (epic #569), per id:
//  - SPELL_WHIRLWIND 38618: the fork commander scripts DoCast 38619 and then (500 ms later) 38618 as
//    SPELL_WHIRLWIND_ADD (src/server/scripts/Northrend/Nexus/Nexus/boss_commander_kolurg.cpp:26-27,160,167
//    and boss_commander_stoutbeard.cpp:26-27,161,168); AC's script casts 38618 only
//    (AC src/server/scripts/Northrend/Nexus/Nexus/boss_commander_stoutbeard_kolurg.cpp:27,60). 38618 is
//    cast in every difficulty (no _H id in either script), so AC's 38618 is kept unchanged.
//  - BUFF_RIFT_SHIELD 47748: the fork Anomalus script applies the same id
//    (.../Nexus/boss_anomalus.cpp:27,159), not difficulty-specific. Unchanged.
//  - GO_CRYSTAL_SPIKE 188537: see OrmorokSpikesTrigger below (fork script never spawns it).
//  No DUNGEON_MODE spell macros exist in the AC Nex files.
enum NexusIDs
{
    // Faction Commander
    NPC_ALLIANCE_COMMANDER          = 27949,
    NPC_HORDE_COMMANDER             = 27947,
    NPC_COMMANDER_STOUTBEARD        = 26796,
    NPC_COMMANDER_KOLURG            = 26798,
    // SPELL_FRIGHTENING_SHOUT         = 19134,
    SPELL_WHIRLWIND                 = 38618,

    // Grand Magus Telestra
    NPC_TELESTRA                    = 26731,
    NPC_FIRE_MAGUS                  = 26928,
    NPC_FROST_MAGUS                 = 26930,
    NPC_ARCANE_MAGUS                = 26929,

    // Anomalus
    BUFF_RIFT_SHIELD                = 47748,

    // Ormorok the Tree Shaper
    // NPC_CRYSTAL_SPIKE               = 27099,
    GO_CRYSTAL_SPIKE                = 188537,
};

// AC Ai/Dungeon/Nex/NexTriggers.h:39
class FactionCommanderWhirlwindTrigger : public Trigger
{
public:
    FactionCommanderWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "faction commander whirlwind") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:46
class TelestraFirebombTrigger : public Trigger
{
public:
    TelestraFirebombTrigger(PlayerbotAI* ai) : Trigger(ai, "telestra firebomb spread") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:53
class TelestraSplitPhaseTrigger : public Trigger
{
public:
    TelestraSplitPhaseTrigger(PlayerbotAI* ai) : Trigger(ai, "telestra split phase") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:60
class ChaoticRiftTrigger : public Trigger
{
public:
    ChaoticRiftTrigger(PlayerbotAI* ai) : Trigger(ai, "chaotic rift") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:67
class OrmorokSpikesTrigger : public Trigger
{
public:
    OrmorokSpikesTrigger(PlayerbotAI* ai) : Trigger(ai, "ormorok spikes") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:74
class OrmorokStackTrigger : public Trigger
{
public:
    OrmorokStackTrigger(PlayerbotAI* ai) : Trigger(ai, "ormorok stack") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:81
class IntenseColdTrigger : public Trigger
{
public:
    IntenseColdTrigger(PlayerbotAI* ai) : Trigger(ai, "intense cold") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/Nex/NexTriggers.h:88
class KeristraszaPositioningTrigger : public Trigger
{
public:
    KeristraszaPositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "keristrasza positioning") {}
    bool IsActive() override;
};

#endif
