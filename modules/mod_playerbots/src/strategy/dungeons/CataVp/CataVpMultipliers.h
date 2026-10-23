/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPMULTIPLIERS_H
#define PLAYERBOTS_CATAVPMULTIPLIERS_H

#include "Multiplier.h"

// While the bot has to get out of (or into) a spot because of one of this dungeon's mechanics, every other movement
// action is vetoed so the usual combat positioning cannot walk it straight back. The state is looked up at most every
// 150 ms: the engine asks a multiplier once per queued action.
// #663 w64: during Grand Vizier Ertan the stack under her is held for the whole fight, also while the bot is in it (flee,
// blink, formation and avoid-aoe moves only ever take it out of the pocket), the same veto set as AC's dungeon multipliers
// (AC Ai/Dungeon/AC/ACMultipliers.cpp:22-31 ShirrakFleeFocusFireMultiplier: AttackAction passes, CastReachTargetSpellAction /
// CastBlinkBackAction / CastDisengageAction and every MovementAction are vetoed); MoP's forward CastBlinkAction is added.
class VpMovementLockMultiplier : public Multiplier
{
public:
    VpMovementLockMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "vp movement lock") {}
    float GetValue(Action* action) override;

private:
    uint32 lastCheck = 0;
    bool locked = false;
    bool ertanStack = false;
};

#endif
