/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ANTRIGGERS_H
#define PLAYERBOTS_ANTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/AN/ANTriggers.h:15
enum AzjolNerubIDs
{
    // Krik'thir the Gatewatcher
    NPC_KRIKTHIR                    = 28684,
    NPC_WATCHER_SILTHIK             = 28731,
    NPC_WATCHER_GASHRA              = 28730,
    NPC_WATCHER_NARJIL              = 28729,
    NPC_WATCHER_SKIRMISHER          = 28734,
    NPC_WATCHER_SHADOWCASTER        = 28733,
    NPC_WATCHER_WARRIOR             = 28732,
    DEBUFF_WEB_WRAP                 = 52086,
    NPC_WEB_WRAP                    = 28619,

    // Anub'arak
    // Not sure how to track this - first one is cast as a buff on himself,
    // which triggers periodic casts of the spikes spell.
    SPELL_IMPALE_PERIODIC           = 53456,
    SPELL_IMPALE_SPIKES             = 53457,
    // 5.4.8 SPELL-ID DECISION (AC ANTriggers.h:33-37 unchanged): the fork's boss script itself switches
    // to the heroic id: DoCastVictim(DUNGEON_MODE(SPELL_POUND, SPELL_POUND_H)) with SPELL_POUND = 53472 and
    // SPELL_POUND_H = 59433 (src/server/scripts/Northrend/AzjolNerub/AzjolNerub/boss_anubarak.cpp:26-27,273),
    // so DUNGEON_MODE is kept (cf. SpellMgr.cpp:471-476).
    SPELL_POUND_N                   = 53472,
    SPELL_POUND_H                   = 59433,
};

#define SPELL_POUND                 DUNGEON_MODE(bot, SPELL_POUND_N, SPELL_POUND_H)

// AC Ai/Dungeon/AN/ANTriggers.h:39
class KrikthirWebWrapTrigger : public Trigger
{
public:
    KrikthirWebWrapTrigger(PlayerbotAI* ai) : Trigger(ai, "krik'thir web wrap") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/AN/ANTriggers.h:46
class KrikthirWatchersTrigger : public Trigger
{
public:
    KrikthirWatchersTrigger(PlayerbotAI* ai) : Trigger(ai, "krik'thir watchers") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/AN/ANTriggers.h:53 (commented out in AC)
// class AnubarakImpaleTrigger : public Trigger
// {
// public:
//     AnubarakImpaleTrigger(PlayerbotAI* ai) : Trigger(ai, "anub'arak impale") {}
//     bool IsActive() override;
// };

// AC Ai/Dungeon/AN/ANTriggers.h:60
class AnubarakPoundTrigger : public Trigger
{
public:
    AnubarakPoundTrigger(PlayerbotAI* ai) : Trigger(ai, "anub'arak pound") {}
    bool IsActive() override;
};

#endif
