/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKTRIGGERS_H
#define PLAYERBOTS_UKTRIGGERS_H

#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum UtgardeKeepIDs
{
    // Prince Keleseth
    SPELL_FROST_TOMB               = 48400,
    NPC_FROST_TOMB                 = 23965,

    // Ingvar the Plunderer
    SPELL_STAGGERING_ROAR_N         = 42708,
    SPELL_STAGGERING_ROAR_H         = 59708,
    SPELL_SMASH_N                   = 42669,
    SPELL_SMASH_H                   = 59706,
    SPELL_DREADFUL_ROAR_N           = 42729,
    SPELL_DREADFUL_ROAR_H           = 59734,
    SPELL_WOE_STRIKE_N              = 42730,
    SPELL_WOE_STRIKE_H              = 59735,
    SPELL_DARK_SMASH_N              = 42723,
    SPELL_DARK_SMASH_H              = 59709,
};

// 5.4.8 ADAPTATION (AC Ai/Dungeon/UK/UKTriggers.h:32-35 uses DUNGEON_MODE(bot, X_N, X_H)):
// on MoP a boss heroic cast keeps the SAME spell id with a difficulty-specific SpellInfo
// (src/server/game/Spells/SpellMgr.cpp:471-476 GetSpellForDifficultyFromSpell returns
// GetSpellInfo(spell->Id, difficulty)), and the fork boss script only ever DoCasts the normal ids in
// every difficulty (src/server/scripts/Northrend/UtgardeKeep/UtgardeKeep/boss_ingvar_the_plunderer.cpp:
// 219 DARK_SMASH, 221 SMASH, 257 DREADFUL_ROAR, 259 STAGGERING_ROAR). The running cast's Id is therefore
// always the _N id, so FindCurrentSpellBySpellId must be given _N; the _H ids stay in the enum for reference.
#define SPELL_STAGGERING_ROAR       SPELL_STAGGERING_ROAR_N
#define SPELL_DREADFUL_ROAR         SPELL_DREADFUL_ROAR_N
#define SPELL_SMASH                 SPELL_SMASH_N
#define SPELL_DARK_SMASH            SPELL_DARK_SMASH_N


// AC Ai/Dungeon/UK/UKTriggers.h class KelesethFrostTombTrigger
class KelesethFrostTombTrigger : public Trigger
{
public:
    KelesethFrostTombTrigger(PlayerbotAI* ai) : Trigger(ai, "keleseth frost tomb") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UK/UKTriggers.h class DalronnDpsTrigger
class DalronnDpsTrigger : public Trigger
{
public:
    DalronnDpsTrigger(PlayerbotAI* ai) : Trigger(ai, "dalronn dps") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UK/UKTriggers.h class IngvarDreadfulRoarTrigger
class IngvarDreadfulRoarTrigger : public Trigger
{
public:
    IngvarDreadfulRoarTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar dreadful roar") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UK/UKTriggers.h class IngvarSmashTankTrigger
class IngvarSmashTankTrigger : public Trigger
{
public:
    IngvarSmashTankTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar smash tank") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UK/UKTriggers.h class IngvarSmashTankReturnTrigger
class IngvarSmashTankReturnTrigger : public Trigger
{
public:
    IngvarSmashTankReturnTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar smash tank return") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UK/UKTriggers.h class NotBehindIngvarTrigger
class NotBehindIngvarTrigger : public Trigger
{
public:
    NotBehindIngvarTrigger(PlayerbotAI* ai) : Trigger(ai, "not behind ingvar") {}
    bool IsActive() override;
};

#endif
