/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/BattlegroundStrategy.h)
 * into the Legends of Azeroth Pandaria fork.
 *
 * Two layers, as upstream has them:
 *   "bg"           - queue and answer invitations. Lives on a bot that is OUT in the world.
 *   "battleground" - objective play, plus one per battleground ("warsong", "arathi", "eye").
 *                    Lives on a bot that is INSIDE a match.
 *
 * AiFactory picks the set, and BGStrategyCheckAction keeps it in step as bots enter and leave.
 */

#ifndef _PLAYERBOT_BGSTRATEGY_H
#define _PLAYERBOT_BGSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

/// Queue for battlegrounds and answer the invitation. Added to random bots' non-combat engine
/// when AiPlayerbot.RandomBotJoinBG is on.
class BgStrategy : public Strategy
{
public:
    BgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "bg"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Objective play shared by every battleground: walk to the start position while the doors are
/// shut, then work the objective the bot picked, re-checking it periodically and on death.
class BattlegroundStrategy : public Strategy
{
public:
    BattlegroundStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "battleground"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Warsong Gulch (and Twin Peaks, which is the same game): flag carriers dominate everything.
class WarsongStrategy : public Strategy
{
public:
    WarsongStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "warsong"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Arathi Basin: capture and hold nodes; no flag carrier to chase.
class ArathiStrategy : public Strategy
{
public:
    ArathiStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "arathi"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Eye of the Storm: nodes plus a single neutral flag either side can carry.
class EyeStrategy : public Strategy
{
public:
    EyeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "eye"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Alterac Valley: nodes, mines, captains and bosses, with the balanced/offensive/defensive strategy
/// roll deciding defender odds (BGTactics::selectObjective's AV case).
class AlteracStrategy : public Strategy
{
public:
    AlteracStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "alterac"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Isle of Conquest (#393): nodes, vehicles and the two keeps. AC-identical class
/// (Ai/Base/Strategy/BattlegroundStrategy.h:72-80); the triggers live in the .cpp.
class IsleStrategy : public Strategy
{
public:
    IsleStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    std::string const getName() override { return "isle"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Strand of the Ancients (#717, map 607): demolishers, cannons, graveyards, seaforium and the relic. No AC spec
/// (AC has no SotA strategy); Arathi's set plus Isle's vehicle weapons, tactics in BgTacticsSA.cpp.
class StrandStrategy : public Strategy
{
public:
    StrandStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    std::string const getName() override { return "strand"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Battle for Gilneas (#576, map 761): Arathi Basin's game on three nodes, so Arathi's trigger set.
/// MoP-only (no AC spec); tactics in BgTacticsBFG.cpp.
class GilneasStrategy : public Strategy
{
public:
    GilneasStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "gilneas"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Temple of Kotmogu (#577, map 998): four orbs carried like a flag. MoP-only; BgTacticsTOK.cpp.
class KotmoguStrategy : public Strategy
{
public:
    KotmoguStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "kotmogu"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Silvershard Mines (#578, map 727): escort / contest the moving mine carts. MoP-only; BgTacticsSM.cpp.
class SilvershardStrategy : public Strategy
{
public:
    SilvershardStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "silvershard"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Deepwind Gorge (#579, map 1105, the 5.3 version): Arathi-style mines plus carts stolen like a flag.
/// MoP-only; BgTacticsDG.cpp.
class DeepwindStrategy : public Strategy
{
public:
    DeepwindStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "deepwind"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Arenas (#346): there is no objective to take, so the objective layer a battleground gets is
/// replaced by ArenaTactics - out of combat, take the class-preferred spot in the middle of the map.
class ArenaStrategy : public Strategy
{
public:
    ArenaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "arena"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
