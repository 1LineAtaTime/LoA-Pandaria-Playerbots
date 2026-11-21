/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots Ai/Base/Strategy/GuildStrategy.h. Dormant as in AC: AiFactory does not add it
 * (the addStrategy("guild") lines are commented out there too); an admin opts in with "+guild" in
 * AiPlayerbot.RandomBotNonCombatStrategies.
 */

#ifndef PLAYERBOTS_GUILDSTRATEGY_H
#define PLAYERBOTS_GUILDSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GuildStrategy : public NonCombatStrategy
{
public:
    GuildStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "guild"; }
    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
