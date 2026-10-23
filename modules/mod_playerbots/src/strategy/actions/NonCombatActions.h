/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/NonCombatActions.h (#149, #527). Execute() carries
 * AC's "food" cheat path (sit + aura 25990, no item needed); the cheat is read from
 * AiPlayerbot.BotCheats inside NonCombatActions.cpp, since this fork has no per-bot cheat mask.
 */

#ifndef _PLAYERBOT_NONCOMBATACTIONS_H
#define _PLAYERBOT_NONCOMBATACTIONS_H

#include "UseItemAction.h"

class PlayerbotAI;

class EatAction : public UseItemAction
{
public:
    EatAction(PlayerbotAI* botAI) : UseItemAction(botAI, "food") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class DrinkAction : public UseItemAction
{
public:
    DrinkAction(PlayerbotAI* botAI) : UseItemAction(botAI, "drink") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

#endif
