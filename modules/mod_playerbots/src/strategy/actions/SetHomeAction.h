/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots Ai/Base/Actions/SetHomeAction.h/.cpp (#611).
 * 5.4.8: HasNpcFlag(UNIT_NPC_FLAG_INNKEEPER), WorldSession::SendBindPoint(Creature*) and
 * Player::GetNPCIfCanInteractWith(guid, flag) all exist on this core with the AC signatures.
 */

#ifndef PLAYERBOTS_SETHOMEACTION_H
#define PLAYERBOTS_SETHOMEACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class SetHomeAction : public MovementAction
{
public:
    SetHomeAction(PlayerbotAI* botAI) : MovementAction(botAI, "home") {}

    bool Execute(Event event) override;
};

#endif
