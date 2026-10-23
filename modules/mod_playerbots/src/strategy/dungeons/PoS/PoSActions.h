/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSACTIONS_H
#define PLAYERBOTS_POSACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PoSTriggers.h"

// AC Ai/Dungeon/PoS/PoSActions.h:16
const Position ICKANDKRICK_TANK_POSITION = Position(816.8508f, 102.331505f, 509.1586f);

// AC Ai/Dungeon/PoS/PoSActions.h:18
class IckAndKrickAction : public AttackAction
{
public:
    IckAndKrickAction(PlayerbotAI* ai) : AttackAction(ai, "ick and krick") {}
    bool Execute(Event event) override;

    bool TankPosition(Unit* boss);
    bool Pursuit(bool pursuit, Unit* boss);
    bool PoisonNova(bool poisonNova, Unit* boss);
    bool ExplosiveBarrage(bool explosiveBarrage, Unit* boss);
};

// AC Ai/Dungeon/PoS/PoSActions.h:30
class TyrannusAction : public AttackAction
{
public:
    TyrannusAction(PlayerbotAI* ai) : AttackAction(ai, "tyrannus") {}
    bool Execute(Event event) override;

    bool RangedSpread(bool rangedSpread);
};
#endif
