/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Value/Stances.h (#177): the "stance" chat command
 * (near/tank/turnback/behind - a manual override of the melee-approach angle, layered the same way
 * Formations.h layers "formation").
 *
 * Header-need workaround: AC stores the bot's chosen Stance as a NamedObjectContext<Value> entry
 * ("stance", Ai/Base/ValueContext.h) so any strategy could read it back with AI_VALUE(Stance*,
 * "stance"). This batch may only touch ActionContext.h/TriggerContext.h/ChatActionContext.h/
 * ChatTriggerContext.h/StrategyContext.h (plus .cpp/new files) - not ValueContext.h - and nothing on
 * this fork besides SetStanceAction itself ever reads a "stance" value (confirmed: AC's own "stance"
 * value has the same single consumer). So the current choice is kept instead as file-static state
 * keyed by bot GUID (StanceChoiceByGuid, in Stances.cpp) and the Stance object is constructed
 * on demand from that name - functionally identical to AC's persisted Stance* for this action's
 * purposes, at the cost of the (unused-by-anything-else) AI_VALUE(Stance*, "stance") lookup path.
 */

#ifndef _PLAYERBOT_STANCES_H
#define _PLAYERBOT_STANCES_H

#include "Action.h"
#include "Formations.h"

class PlayerbotAI;
class Unit;

class Stance : public Formation
{
public:
    Stance(PlayerbotAI* botAI, std::string const name) : Formation(botAI, name) {}

    WorldLocation GetLocation() override;
    std::string const GetTargetName() override;
    float GetMaxDistance() override;

protected:
    virtual Unit* GetTarget();
    virtual WorldLocation GetLocationInternal() = 0;
    virtual WorldLocation GetNearLocation(float angle, float distance);
};

class MoveStance : public Stance
{
public:
    MoveStance(PlayerbotAI* botAI, std::string const name) : Stance(botAI, name) {}

protected:
    WorldLocation GetLocationInternal() override;
    virtual float GetAngle() = 0;
};

class SetStanceAction : public Action
{
public:
    SetStanceAction(PlayerbotAI* botAI) : Action(botAI, "set stance") {}

    bool Execute(Event event) override;
};

#endif
