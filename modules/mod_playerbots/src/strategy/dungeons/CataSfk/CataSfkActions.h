/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#ifndef PLAYERBOTS_CATASFKACTIONS_H
#define PLAYERBOTS_CATASFKACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MovementActions.h"

// Base of every movement action of this strategy: the avoid multiplier lets exactly these through while a bot is in
// danger (the same dynamic_cast idiom as HoSMultipliers.cpp:23-27).
class SfkAvoidAction : public MovementAction
{
public:
    SfkAvoidAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}
};

class SfkInterruptPainAndSufferingAction : public Action
{
public:
    SfkInterruptPainAndSufferingAction(PlayerbotAI* botAI) : Action(botAI, "sfk interrupt pain and suffering") {}
    bool Execute(Event event) override;
};

class SfkKillWorgenSpiritAction : public AttackAction
{
public:
    SfkKillWorgenSpiritAction(PlayerbotAI* botAI) : AttackAction(botAI, "sfk kill worgen spirit") {}
    bool Execute(Event event) override;
};

class SfkInterruptWorgenAction : public Action
{
public:
    SfkInterruptWorgenAction(PlayerbotAI* botAI) : Action(botAI, "sfk interrupt worgen") {}
    bool Execute(Event event) override;
};

class SfkLeaveDesecrationAction : public SfkAvoidAction
{
public:
    SfkLeaveDesecrationAction(PlayerbotAI* botAI) : SfkAvoidAction(botAI, "sfk leave desecration") {}
    bool Execute(Event event) override;
};

class SfkInterruptEmpowermentAction : public Action
{
public:
    SfkInterruptEmpowermentAction(PlayerbotAI* botAI) : Action(botAI, "sfk interrupt empowerment") {}
    bool Execute(Event event) override;
};

class SfkLeaveShieldConeAction : public SfkAvoidAction
{
public:
    SfkLeaveShieldConeAction(PlayerbotAI* botAI) : SfkAvoidAction(botAI, "sfk leave shield cone") {}
    bool Execute(Event event) override;
};

class SfkLeavePoisonMixtureAction : public SfkAvoidAction
{
public:
    SfkLeavePoisonMixtureAction(PlayerbotAI* botAI) : SfkAvoidAction(botAI, "sfk leave poison mixture") {}
    bool Execute(Event event) override;
};

class SfkKeepMovingAction : public SfkAvoidAction
{
public:
    SfkKeepMovingAction(PlayerbotAI* botAI) : SfkAvoidAction(botAI, "sfk keep moving") {}
    bool Execute(Event event) override;
};

// AC HodirBitingColdJumpAction (UldActions.h:202-209): a MovementAction that only clears the stacks
class SfkCoagulantJumpAction : public SfkAvoidAction
{
public:
    SfkCoagulantJumpAction(PlayerbotAI* botAI) : SfkAvoidAction(botAI, "sfk coagulant jump") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SfkInterruptCursedBulletsAction : public Action
{
public:
    SfkInterruptCursedBulletsAction(PlayerbotAI* botAI) : Action(botAI, "sfk interrupt cursed bullets") {}
    bool Execute(Event event) override;
};

class SfkLeaveBarrageConeAction : public SfkAvoidAction
{
public:
    SfkLeaveBarrageConeAction(PlayerbotAI* botAI) : SfkAvoidAction(botAI, "sfk leave barrage cone") {}
    bool Execute(Event event) override;
};

class SfkKillGhoulAction : public AttackAction
{
public:
    SfkKillGhoulAction(PlayerbotAI* botAI) : AttackAction(botAI, "sfk kill ghoul") {}
    bool Execute(Event event) override;
};


#endif
