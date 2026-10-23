/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Action/RpgAction.h)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 */

#ifndef PLAYERBOTS_RPGACTION_H
#define PLAYERBOTS_RPGACTION_H

#include "MovementActions.h"

class ObjectGuid;
class PlayerbotAI;
class Unit;
class WorldObject;

// 5.4.8 adaptation: AC calls chat->FormatWorldobject(wo) (AC Bot/Cmd/ChatHelper.cpp:387-396). This fork's
// ChatHelper (AI/ChatHelper.h) has no FormatWorldobject and ChatHelper.h is not editable in this lane, so
// the body is copied here verbatim as a free function. Defined in RpgAction.cpp. If ChatHelper::FormatWorldobject
// is ported later, every RpgFormatWorldobject(...) call in the rpg files can become chat->FormatWorldobject(...).
std::string const RpgFormatWorldobject(WorldObject* wo);

class RpgAction : public MovementAction
{
public:
    RpgAction(PlayerbotAI* botAI, std::string const name = "rpg") : MovementAction(botAI, name) { }
    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    virtual bool SetNextRpgAction();
    typedef void (RpgAction::*RpgElement)(ObjectGuid guid);
    bool AddIgnore(ObjectGuid guid);
    bool RemIgnore(ObjectGuid guid);
    bool HasIgnore(ObjectGuid guid);
};

class CRpgAction : public RpgAction
{
public:
    CRpgAction(PlayerbotAI* botAI) : RpgAction(botAI, "crpg") { }
    bool isUseful() override;
};

#endif
