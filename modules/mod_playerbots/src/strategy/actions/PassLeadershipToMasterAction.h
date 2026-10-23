/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/PassLeadershipToMasterAction.h (#177): the "give
 * leader" chat command, plus its PassLeadershipToMasterAction base (used only by GiveLeaderAction on
 * this fork - see the class comment in the .cpp for what AC's own automatic self-trigger needed that
 * this fork does not have).
 */

#ifndef _PLAYERBOT_PASSLEADERSHIPTOMASTERACTION_H
#define _PLAYERBOT_PASSLEADERSHIPTOMASTERACTION_H

#include "Action.h"

class PlayerbotAI;

class PassLeadershipToMasterAction : public Action
{
public:
    PassLeadershipToMasterAction(PlayerbotAI* botAI, std::string const name = "leader",
                                 std::string const message = "Passing leader to you!")
        : Action(botAI, name), message(message)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    std::string const message;
};

class GiveLeaderAction : public PassLeadershipToMasterAction
{
public:
    GiveLeaderAction(PlayerbotAI* botAI, std::string const message = "Lead the way!")
        : PassLeadershipToMasterAction(botAI, "give leader", message)
    {
    }

    bool isUseful() override;
};

#endif
