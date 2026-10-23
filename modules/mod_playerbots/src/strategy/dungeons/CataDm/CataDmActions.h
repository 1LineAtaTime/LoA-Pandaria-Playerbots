/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#ifndef PLAYERBOTS_CATADMACTIONS_H
#define PLAYERBOTS_CATADMACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MovementActions.h"

// Base of every movement action of this strategy: the avoid multiplier lets exactly these through while a bot is in
// danger (the same dynamic_cast idiom as HoSMultipliers.cpp:23-27).
class DmAvoidAction : public MovementAction
{
public:
    DmAvoidAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // #667 w68: walk to one spot clear of every bomb hazard (CataDm::BombEscapeSpot). 1 = walk started, 0 = our own escape walk
    // is still running towards a spot that is still safe (wait for it), -1 = no better spot (the caller falls back).
    int EscapeBombs();

private:
    bool _escaping = false;
    float _escX = 0.0f;
    float _escY = 0.0f;
};

class DmLeaveBlossomAction : public DmAvoidAction
{
public:
    DmLeaveBlossomAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm leave blossom") {}
    bool Execute(Event event) override;
};

class DmKillBlossomAction : public AttackAction
{
public:
    DmKillBlossomAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm kill blossom") {}
    bool Execute(Event event) override;
};

class DmLeaveStickyBombAction : public DmAvoidAction
{
public:
    DmLeaveStickyBombAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm leave sticky bomb") {}
    bool Execute(Event event) override;
};

class DmSpreadChestBombAction : public DmAvoidAction
{
public:
    DmSpreadChestBombAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm spread chest bomb") {}
    bool Execute(Event event) override;
};

class DmAttackHelixAction : public AttackAction
{
public:
    DmAttackHelixAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm attack helix") {}
    bool Execute(Event event) override;
};

class DmLeaveHarvestAction : public DmAvoidAction
{
public:
    DmLeaveHarvestAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm leave harvest") {}
    bool Execute(Event event) override;
};

class DmLeaveOverdriveAction : public DmAvoidAction
{
public:
    DmLeaveOverdriveAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm leave overdrive") {}
    bool Execute(Event event) override;
};

class DmKillVaporAction : public AttackAction
{
public:
    DmKillVaporAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm kill vapor") {}
    bool Execute(Event event) override;
};

class DmKillVanessaAddAction : public AttackAction
{
public:
    DmKillVanessaAddAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm kill vanessa add") {}
    bool Execute(Event event) override;
};

// ---- #681 / #826 (see CataDmShared.h) ----
class DmDodgeFireWallAction : public DmAvoidAction
{
public:
    DmDodgeFireWallAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm dodge fire wall") {}
    bool Execute(Event event) override;
};

class DmAvoidMoltenSlagAction : public DmAvoidAction
{
public:
    DmAvoidMoltenSlagAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm avoid molten slag") {}
    bool Execute(Event event) override;
};

class DmKillMoltenSlagAction : public AttackAction
{
public:
    DmKillMoltenSlagAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm kill molten slag") {}
    bool Execute(Event event) override;
};

class DmAttackReaperAction : public AttackAction
{
public:
    DmAttackReaperAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm attack reaper") {}
    bool Execute(Event event) override;
};

class DmEscapeBlazeAction : public DmAvoidAction
{
public:
    DmEscapeBlazeAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm escape blaze") {}
    bool Execute(Event event) override;
};

class DmLeavePowderKegAction : public DmAvoidAction
{
public:
    DmLeavePowderKegAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm leave powder keg") {}
    bool Execute(Event event) override;
};

class DmClickSteamValveAction : public Action
{
public:
    DmClickSteamValveAction(PlayerbotAI* botAI) : Action(botAI, "dm click steam valve") {}
    bool Execute(Event event) override;
};

class DmEscapeNightmareHazardAction : public DmAvoidAction
{
public:
    DmEscapeNightmareHazardAction(PlayerbotAI* botAI) : DmAvoidAction(botAI, "dm escape nightmare hazard") {}
    bool Execute(Event event) override;
};

class DmSaveHarringtonAction : public AttackAction
{
public:
    DmSaveHarringtonAction(PlayerbotAI* botAI) : AttackAction(botAI, "dm save harrington") {}
    bool Execute(Event event) override;
};


#endif
