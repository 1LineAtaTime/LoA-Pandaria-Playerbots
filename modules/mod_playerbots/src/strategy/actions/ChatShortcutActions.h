/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ChatShortcutActions.h. #173 ported "follow"/"stay"/
 * "move from group"; #177 completes the set with flee/runaway/grind/tank attack/max dps (the
 * "movement/state" row of group-functionality-parity.md section 3.1a). AC's Naxx/BWL shortcuts are
 * left out: those are AC-specific raid-strategy toggles for content this fork does not have.
 */

#ifndef _PLAYERBOT_CHATSHORTCUTACTIONS_H
#define _PLAYERBOT_CHATSHORTCUTACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;

class PositionsResetAction : public Action
{
public:
    PositionsResetAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

    void ResetReturnPosition();
    void SetReturnPosition(float x, float y, float z);
    void ResetStayPosition();
    void SetStayPosition(float x, float y, float z);
};

class FollowChatShortcutAction : public MovementAction
{
public:
    FollowChatShortcutAction(PlayerbotAI* botAI) : MovementAction(botAI, "follow chat shortcut") {}

    bool Execute(Event event) override;
};

class StayChatShortcutAction : public PositionsResetAction
{
public:
    StayChatShortcutAction(PlayerbotAI* botAI) : PositionsResetAction(botAI, "stay chat shortcut") {}

    bool Execute(Event event) override;
};

class MoveFromGroupChatShortcutAction : public Action
{
public:
    MoveFromGroupChatShortcutAction(PlayerbotAI* botAI) : Action(botAI, "move from group chat shortcut") {}

    bool Execute(Event event) override;
};

class FleeChatShortcutAction : public PositionsResetAction
{
public:
    FleeChatShortcutAction(PlayerbotAI* botAI) : PositionsResetAction(botAI, "flee chat shortcut") {}

    bool Execute(Event event) override;
};

class GoawayChatShortcutAction : public PositionsResetAction
{
public:
    GoawayChatShortcutAction(PlayerbotAI* botAI) : PositionsResetAction(botAI, "runaway chat shortcut") {}

    bool Execute(Event event) override;
};

class GrindChatShortcutAction : public PositionsResetAction
{
public:
    GrindChatShortcutAction(PlayerbotAI* botAI) : PositionsResetAction(botAI, "grind chat shortcut") {}

    bool Execute(Event event) override;
};

class TankAttackChatShortcutAction : public PositionsResetAction
{
public:
    TankAttackChatShortcutAction(PlayerbotAI* botAI) : PositionsResetAction(botAI, "tank attack chat shortcut") {}

    bool Execute(Event event) override;
};

class MaxDpsChatShortcutAction : public Action
{
public:
    MaxDpsChatShortcutAction(PlayerbotAI* botAI) : Action(botAI, "max dps chat shortcut") {}

    bool Execute(Event event) override;
};

#endif
