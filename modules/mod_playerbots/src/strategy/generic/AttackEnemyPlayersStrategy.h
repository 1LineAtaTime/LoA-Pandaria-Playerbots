/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/AttackEnemyPlayersStrategy).
 */

#ifndef _PLAYERBOT_ATTACKENEMYPLAYERSSTRATEGY_H
#define _PLAYERBOT_ATTACKENEMYPLAYERSSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

/// Fires "attack enemy player" when a hostile player is in range. Lives on the non-combat engine so
/// a bot engages a flagged enemy before it is itself attacked. The strategy name is "pvp".
class AttackEnemyPlayersStrategy : public NonCombatStrategy
{
public:
    AttackEnemyPlayersStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "pvp"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
