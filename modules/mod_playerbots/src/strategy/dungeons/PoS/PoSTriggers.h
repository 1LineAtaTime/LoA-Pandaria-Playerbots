/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSTRIGGERS_H
#define PLAYERBOTS_POSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/PoS/PoSTriggers.h:15
enum PitOfSaronIDs
{
    // 5.4.8 SPELL-ID RULE (epic #569): AC has no DUNGEON_MODE on any PoS id. Every id below that the strategy
    // compares against is the same id the fork's boss script casts, in every difficulty
    // (src/server/scripts/Northrend/FrozenHalls/PitOfSaron/boss_krickandick.cpp:27-30,33,259-260,269,274,341);
    // a heroic cast keeps the same id with a difficulty SpellInfo (src/server/game/Spells/SpellMgr.cpp:471-476).
    // NPC entries match the fork's pit_of_saron.h:45-49 and the MoP/AC creature_template.

    //NPCs
    NPC_GARFROST                        = 36494,
    NPC_KRICK                           = 36477,
    NPC_ICK                             = 36476,
    NPC_TYRANNUS                        = 36658,
    NPC_RIMEFANG                        = 36661,

    //Spells
    // SpellEffect.dbc: 68987 effect 0 applies aura 4 (dummy) to the target, so bot->HasAura works.
    SPELL_PURSUIT                       = 68987,
    // The fork casts only 68989 (boss_krickandick.cpp:29,269); the heroic id 70434 is never cast by the fork's
    // script. AC compares both as plain ids (PoSActions.cpp:32, PoSMultipliers.cpp:24), so both are kept.
    SPELL_POISON_NOVA_POS               = 68989,
    SPELL_POISON_NOVA_POS_HC            = 70434,
    SPELL_EXPLOSIVE_BARRAGE_KRICK       = 69012,  // boss_krickandick.cpp:27,259
    SPELL_EXPLOSIVE_BARRAGE_ICK         = 69263,  // boss_krickandick.cpp:28,260
    SPELL_EXPLOSIVE_BARRAGE_SUMMON      = 69015,
    // Aura on the exploding orbs (boss_krickandick.cpp:33,341); SpellEffect.dbc: 69017 effect 0 is aura 4 (dummy).
    SPELL_EXPLODING_ORB_VISUAL          = 69017,
    SPELL_MARK_OF_RIMEFANG              = 69275,
    RIMEFANG_SPELL_HOARFROST            = 69246,
    RIMEFANG_SPELL_HOARFROST_HC         = 69245,
    RIMEFANG_SPELL_HOARFROST_HC2        = 69645,
};

// AC Ai/Dungeon/PoS/PoSTriggers.h:38
class IckAndKrickTrigger : public Trigger
{
public:
    IckAndKrickTrigger(PlayerbotAI* ai) : Trigger(ai, "ick and krick") {}

    bool IsActive() override;
};

// AC Ai/Dungeon/PoS/PoSTriggers.h:46
class TyrannusTrigger : public Trigger
{
public:
    TyrannusTrigger(PlayerbotAI* ai) : Trigger(ai, "tyrannus") {}

    bool IsActive() override;
};

#endif
