/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKTRIGGERS_H
#define PLAYERBOTS_DTKTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/DTK/DTKTriggers.h:15 enum DrakTharonIDs
// 5.4.8 SPELL-ID RULE (epic #569), per id (AC has no DUNGEON_MODE spell macros in DTK):
//  - SPELL_ARCANE_FIELD 47346: the fork Novos script DoCasts 47346 in every difficulty
//    (src/server/scripts/Northrend/DraktharonKeep/boss_novos.cpp:39,273). Unchanged.
//  - SPELL_GIFT_OF_THARONJA 52509: the fork Tharon'ja script casts 52509 (boss_tharon_ja.cpp:29,199) and the
//    MoP SpellEffect.dbc (Id 52509, 3 effects, aura names 36/22/260 on target type 22) still applies it as an
//    aura to the players, so bot->HasAura(52509) holds. Unchanged.
//  - NPC_DRAKKARI_INVADER 27709 is the fork's NPC_DRAKKARI_INVADER_A (boss_trollgore.cpp:55); the fork also
//    has _B 27753 / _C 27754 (lines 56-57), AC's check covers 27709 only and is kept as AC.
enum DrakTharonIDs
{
    // Trollgore
    NPC_DRAKKARI_INVADER               = 27709,

    // Novos the Summoner
    NPC_NOVOS                          = 26631,
    SPELL_ARCANE_FIELD                 = 47346,
    NPC_CRYSTAL_HANDLER                = 26627,
    NPC_HULKING_CORPSE                 = 27597,
    NPC_RISEN_SHADOWCASTER             = 27600,
    NPC_FETID_TROLL_CORPSE             = 27598,

    // The Prophet Tharon'ja
    SPELL_GIFT_OF_THARONJA             = 52509,
};

// AC Ai/Dungeon/DTK/DTKTriggers.h:32
class CorpseExplodeTrigger : public Trigger
{
public:
    CorpseExplodeTrigger(PlayerbotAI* ai) : Trigger(ai, "corpse explode") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/DTK/DTKTriggers.h:39
class ArcaneFieldTrigger : public Trigger
{
public:
    ArcaneFieldTrigger(PlayerbotAI* ai) : Trigger(ai, "arcane field") {}
    bool IsActive() override;
};

// class CrystalHandlerTrigger : public Trigger
// {
// public:
//     CrystalHandlerTrigger(PlayerbotAI* ai) : Trigger(ai, "crystal handler") {}
//     bool IsActive() override;
// };

// AC Ai/Dungeon/DTK/DTKTriggers.h:53
class GiftOfTharonjaTrigger : public Trigger
{
public:
    GiftOfTharonjaTrigger(PlayerbotAI* ai) : Trigger(ai, "gift of tharon'ja") {}
    bool IsActive() override;
};

#endif
