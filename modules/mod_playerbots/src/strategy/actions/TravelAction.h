/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/TravelAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #269.
 */

#ifndef _PLAYERBOT_TRAVELACTION_H
#define _PLAYERBOT_TRAVELACTION_H

#include "MovementActions.h"

class PlayerbotAI;

// All four AC classes of Ai/Base/Actions/TravelAction.h are ported (class order as AC). TravelAction
// reads the TravelTarget class (Travel/TravelMgr.h, #268); MoveToDarkPortalAction needs
// RandomPlayerbotMgr::GetCreatureDataByEntry, which the fork does not have, so TravelAction.cpp carries a
// file-local static helper with the same body (see there).

class TravelAction : public MovementAction
{
public:
    TravelAction(PlayerbotAI* botAI) : MovementAction(botAI, "travel") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveToDarkPortalAction : public MovementAction
{
public:
    MoveToDarkPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to dark portal") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DarkPortalAzerothAction : public MovementAction
{
public:
    DarkPortalAzerothAction(PlayerbotAI* botAI) : MovementAction(botAI, "dark portal azeroth") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveFromDarkPortalAction : public MovementAction
{
public:
    MoveFromDarkPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "move from dark portal") {}

    bool Execute(Event event) override;
};

#endif