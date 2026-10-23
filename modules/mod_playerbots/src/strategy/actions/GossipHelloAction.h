/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/GossipHelloAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #258 slice "Trainer + GossipHello" (the NPC interaction layer).
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h, WorldPacketActionContext.h and
 * WorldPacketHandlerStrategy.cpp are owned by the single later registration slice of #258.
 */

#ifndef _PLAYERBOT_GOSSIPHELLOACTION_H
#define _PLAYERBOT_GOSSIPHELLOACTION_H

#include "Action.h"

class ObjectGuid;  // `class`, not `struct`: src/server/game/Entities/Object/ObjectGuid.h:135 declares it as a
                   // class, and a mismatched key here is an LNK2019 for a symbol that does exist.

class PlayerbotAI;

class GossipHelloAction : public Action
{
public:
    GossipHelloAction(PlayerbotAI* botAI) : Action(botAI, "gossip hello") {}

    bool Execute(Event event) override;
    // Overload for direct usage
    bool Execute(ObjectGuid guid, int32 menuToSelect, bool silent = false);

private:
    void TellGossipMenus();
    bool ProcessGossip(int32 menuToSelect, bool silent);
    void TellGossipText(uint32 textId);
};

#endif