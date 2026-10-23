/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#ifndef PLAYERBOTS_MOPSCHOLOACTIONS_H
#define PLAYERBOTS_MOPSCHOLOACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MopScholoShared.h"
#include "MovementActions.h"

// Leave the nearest live ground-effect carrier of one hazard type (flee to its radius + margin).
class MopScholoLeaveHazardAction : public MovementAction
{
public:
    MopScholoLeaveHazardAction(PlayerbotAI* botAI, std::string const name, MopScholomance::Hazard const& hazard)
        : MovementAction(botAI, name), _hazard(hazard)
    {
    }

    bool Execute(Event event) override;

private:
    MopScholomance::Hazard const& _hazard;
};

// Step away from the nearest group member until `spread` yards separate them.
class MopScholoSpreadAction : public MovementAction
{
public:
    MopScholoSpreadAction(PlayerbotAI* botAI, std::string const name, float spread)
        : MovementAction(botAI, name), _spread(spread)
    {
    }

    bool Execute(Event event) override;

private:
    float const _spread;
};

class MopScholoFixatedAction : public MovementAction
{
public:
    MopScholoFixatedAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo run from soul") {}
    bool Execute(Event event) override;
};

class MopScholoInterruptAction : public Action
{
public:
    MopScholoInterruptAction(PlayerbotAI* botAI) : Action(botAI, "scholo priority interrupt") {}
    bool Execute(Event event) override;
};

class MopScholoFocusAction : public AttackAction
{
public:
    MopScholoFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "scholo focus target") {}
    bool Execute(Event event) override;
};

// Walk to the nearest Bone Pile and click it (spellclick) for Bone Armor.
// #781 Jandice's Wondrous Rapidity (AC Ai/Dungeon/UK/UKActions.h:31-45, IngvarDodgeSmashAction /
// IngvarSmashReturnAction): the tank walks through her to her back while the cone is up, then back to her front.
class MopScholoDodgeRapidityAction : public MovementAction
{
public:
    MopScholoDodgeRapidityAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo dodge rapidity") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MopScholoRapidityReturnAction : public MovementAction
{
public:
    MopScholoRapidityReturnAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo rapidity return") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// #659 Chillheart's Ice Wave: walk ahead of the wall (+Y) to IceWallSafeY.
class MopScholoOutrunIceWallAction : public MovementAction
{
public:
    MopScholoOutrunIceWallAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo outrun ice wall") {}
    bool Execute(Event event) override;
};

// #659: the tank walks its boss to the fight spot (Chillheart: landing / corridor mouth; Jandice: passage mouth).
class MopScholoTankSpotAction : public MovementAction
{
public:
    MopScholoTankSpotAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo tank spot") {}
    bool Execute(Event event) override;
};

// #806 Harsh Lesson: a healer dispels Explosive Pain off the Fresh Test Subject whose blast hits the most Expired ones.
class MopScholoHarshLessonAction : public Action
{
public:
    MopScholoHarshLessonAction(PlayerbotAI* botAI) : Action(botAI, "scholo harsh lesson") {}
    bool Execute(Event event) override;
};

// #806: the classroom is clear and its door open: walk back to Gandling.
class MopScholoLessonExitAction : public MovementAction
{
public:
    MopScholoLessonExitAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo lesson exit") {}
    bool Execute(Event event) override;
};

class MopScholoBoneArmorAction : public MovementAction
{
public:
    MopScholoBoneArmorAction(PlayerbotAI* botAI) : MovementAction(botAI, "scholo get bone armor") {}
    bool Execute(Event event) override;
};

#endif
