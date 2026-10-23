/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LEAVEGROUPACTION_H
#define _PLAYERBOT_LEAVEGROUPACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;

class LeaveGroupAction : public Action
{
public:
    LeaveGroupAction(PlayerbotAI* botAI, std::string const name = "leave") : Action(botAI, name) {}

    bool Execute(Event event) override;

    virtual bool Leave(Player* player);
};

class UninviteAction : public LeaveGroupAction
{
public:
    UninviteAction(PlayerbotAI* botAI) : LeaveGroupAction(botAI, "uninvite") {}

    bool Execute(Event event) override;
};

/// Ported from AC mod-playerbots LeaveGroupAction.h:34-38 (#178/#179). Reacts to SMSG_PARTY_COMMAND_RESULT
/// so a random bot picks a new master (or leaves) the instant its master leaves the party, instead of
/// waiting for the next "master reassignment" pass already done in PlayerbotAI::UpdateAI (see
/// LeaveGroupAction.cpp for the .cpp-local reimplementation of AC's PlayerbotAI::FindNewMaster(), which
/// this fork does not expose as a public header member).
class PartyCommandAction : public LeaveGroupAction
{
public:
    PartyCommandAction(PlayerbotAI* botAI) : LeaveGroupAction(botAI, "party command") {}

    bool Execute(Event event) override;
};

/// Ported from AC mod-playerbots LeaveGroupAction.h:41 / .cpp:98-161 (#149). AC's isUseful() reads
/// botAI->GetGroupLeader(), which does not exist on this fork's PlayerbotAI (only GetMaster()
/// does); adapted to use bot->GetGroup()/Group::GetLeaderGUID(), the same pattern already used by
/// BattleGroundJoinAction.cpp:84 to check group leadership. AC's Leave() takes no parameter; this
/// fork's LeaveGroupAction::Leave() takes the Player* to notify, so Execute() calls Leave(bot).
/// AC's isUseful() also branches on several AC-only concepts this fork does not have
/// (GetGrouperType()/GrouperType::SOLO, IsAltBot(), IsSelfBot(), HasGameClientMaster(),
/// "should get money") - those are dropped, keeping the map/level/distance "far away" checks that
/// give the action its name.
class LeaveFarAwayAction : public LeaveGroupAction
{
public:
    LeaveFarAwayAction(PlayerbotAI* botAI) : LeaveGroupAction(botAI, "leave far away") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
