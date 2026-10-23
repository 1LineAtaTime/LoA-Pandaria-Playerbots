/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#ifndef PLAYERBOTS_CATALCTACTIONS_H
#define PLAYERBOTS_CATALCTACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MovementActions.h"

// Base of every movement action of this strategy: the avoid multiplier lets exactly these through while a bot is in
// danger (the same dynamic_cast idiom as HoSMultipliers.cpp:23-27).
class LctAvoidAction : public MovementAction
{
public:
    LctAvoidAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}
};

class LctAvoidLandMineAction : public LctAvoidAction
{
public:
    LctAvoidLandMineAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid land mine") {}
    bool Execute(Event event) override;
};

class LctAvoidShockwaveAction : public LctAvoidAction
{
public:
    LctAvoidShockwaveAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid shockwave") {}
    bool Execute(Event event) override;
};

class LctDropFromWallAction : public LctAvoidAction
{
public:
    LctDropFromWallAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct drop from wall") {}
    bool Execute(Event event) override;
};

// #820: not a MovementAction move - a teleport (AC ICC gunship / Karazhan Nightbane pattern), so it derives from Action
class LctReturnToPlatformAction : public Action
{
public:
    LctReturnToPlatformAction(PlayerbotAI* botAI) : Action(botAI, "lct return to platform") {}
    bool Execute(Event event) override;

private:
    uint32 _lastReturn = 0;
};

class LctLeaveDustFlailAction : public LctAvoidAction
{
public:
    LctLeaveDustFlailAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct leave dust flail") {}
    bool Execute(Event event) override;
};

class LctKillCrocoliskAction : public AttackAction
{
public:
    LctKillCrocoliskAction(PlayerbotAI* botAI) : AttackAction(botAI, "lct kill crocolisk") {}
    bool Execute(Event event) override;
};

class LctAvoidHeavensFuryAction : public LctAvoidAction
{
public:
    LctAvoidHeavensFuryAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid heavens fury") {}
    bool Execute(Event event) override;
};

class LctKillSoulFragmentAction : public AttackAction
{
public:
    LctKillSoulFragmentAction(PlayerbotAI* botAI) : AttackAction(botAI, "lct kill soul fragment") {}
    bool Execute(Event event) override;
};

class LctAvoidHallowedGroundAction : public LctAvoidAction
{
public:
    LctAvoidHallowedGroundAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid hallowed ground") {}
    bool Execute(Event event) override;
};

class LctKillHarbingerAction : public AttackAction
{
public:
    LctKillHarbingerAction(PlayerbotAI* botAI) : AttackAction(botAI, "lct kill harbinger") {}
    bool Execute(Event event) override;
};

class LctAvoidBlazeFireAction : public LctAvoidAction
{
public:
    LctAvoidBlazeFireAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid blaze fire") {}
    bool Execute(Event event) override;
};

class LctAvoidSiamatStormAction : public LctAvoidAction
{
public:
    LctAvoidSiamatStormAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid siamat storm") {}
    bool Execute(Event event) override;
};

class LctKillServantAction : public AttackAction
{
public:
    LctKillServantAction(PlayerbotAI* botAI) : AttackAction(botAI, "lct kill servant") {}
    bool Execute(Event event) override;
};

class LctAvoidLightningChargeAction : public LctAvoidAction
{
public:
    LctAvoidLightningChargeAction(PlayerbotAI* botAI) : LctAvoidAction(botAI, "lct avoid lightning charge") {}
    bool Execute(Event event) override;
};


#endif
