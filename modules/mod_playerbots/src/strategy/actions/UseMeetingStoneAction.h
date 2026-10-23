/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UseMeetingStoneAction.h (#177): the "summon" chat
 * command (SummonAction) plus its UseMeetingStoneAction sibling (a real meeting-stone GameObject
 * click summoning the bot). See the .cpp for what UseMeetingStoneAction still needs before it can be
 * registered.
 */

#ifndef _PLAYERBOT_USEMEETINGSTONEACTION_H
#define _PLAYERBOT_USEMEETINGSTONEACTION_H

#include "MovementActions.h"

class Player;
class PlayerbotAI;

class SummonAction : public MovementAction
{
public:
    SummonAction(PlayerbotAI* botAI, std::string const name = "summon") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool Teleport(Player* summoner, Player* player, bool preserveAuras);

protected:
    bool SummonUsingGos(Player* summoner, Player* player, bool preserveAuras);
    bool SummonUsingNpcs(Player* summoner, Player* player, bool preserveAuras);
};

// Not yet registered anywhere - see the .cpp file comment (#177 report: "riskiest spots").
class UseMeetingStoneAction : public SummonAction
{
public:
    UseMeetingStoneAction(PlayerbotAI* botAI) : SummonAction(botAI, "use meeting stone") {}

    bool Execute(Event event) override;
};

#endif
