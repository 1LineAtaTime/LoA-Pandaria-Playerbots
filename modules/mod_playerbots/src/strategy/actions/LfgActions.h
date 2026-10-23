/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/LfgActions.h) into the
 * Legends of Azeroth Pandaria fork (#210 S10 G1-G3). See LfgActions.cpp for the full port notes.
 * #142: LfgJoinAction/LfgLeaveAction (AC's random-bot self-queue, Ai/Base/Actions/LfgActions.h:14-56) are ported below.
 */

#ifndef _PLAYERBOT_LFGACTIONS_H
#define _PLAYERBOT_LFGACTIONS_H

#include "Action.h"

class PlayerbotAI;

// #142 (AC LfgActions.h:14-25): "lfg join" - a random bot queues itself for the dungeons real players
// are queued for (RandomPlayerbotMgr::LfgDungeons). AC derives it from InventoryAction (only for a
// commented-out gear-score block) and derives LfgAccept/LfgRoleCheck from it to share GetRoles(); this
// fork keeps those two as plain Actions with the free GetLfgRoles() in LfgActions.cpp.
class LfgJoinAction : public Action
{
public:
    LfgJoinAction(PlayerbotAI* botAI, std::string const name = "lfg join") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    bool JoinLFG();
};

// #142 (AC LfgActions.h:42-49): "lfg leave".
class LfgLeaveAction : public Action
{
public:
    LfgLeaveAction(PlayerbotAI* botAI) : Action(botAI, "lfg leave") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class LfgRoleCheckAction : public Action{
public:
    LfgRoleCheckAction(PlayerbotAI* botAI) : Action(botAI, "lfg role check") {}

    bool Execute(Event event) override;
    bool isUseful() override { return true; }
};

class LfgAcceptAction : public Action
{
public:
    LfgAcceptAction(PlayerbotAI* botAI) : Action(botAI, "lfg accept") {}

    bool Execute(Event event) override;
    bool isUseful() override { return true; }
};

class LfgTeleportAction : public Action
{
public:
    LfgTeleportAction(PlayerbotAI* botAI) : Action(botAI, "lfg teleport") {}

    bool Execute(Event event) override;
    bool isUseful() override { return true; }
};

#endif
