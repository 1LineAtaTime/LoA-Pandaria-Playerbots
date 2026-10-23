/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/RevealGatheringItemAction + the
 * gather strategy) into the Legends of Azeroth Pandaria fork. The upstream version tracks gathering
 * items with a dedicated value and moves the bot with the travel system; this fork takes the simple
 * case a random bot needs: find a herb/mining node it has the skill for, walk to it and harvest it
 * (issue #62).
 */

#ifndef _PLAYERBOT_GATHERACTION_H
#define _PLAYERBOT_GATHERACTION_H

#include "MovementActions.h"

class GameObject;
class PlayerbotAI;

class GatherAction : public MovementAction
{
public:
    GatherAction(PlayerbotAI* botAI) : MovementAction(botAI, "gather") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    /// Nearest herb/mining node the bot has the skill to harvest (within a short walk).
    GameObject* FindGatherableNode();

    GameObject* _node{nullptr};
};

#endif
