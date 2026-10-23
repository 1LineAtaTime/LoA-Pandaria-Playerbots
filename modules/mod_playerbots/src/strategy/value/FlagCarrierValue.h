/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/PvpValues.h) into the
 * Legends of Azeroth Pandaria fork.
 *
 * "enemy flag carrier" and "team flag carrier" were referenced by ChooseTargetActions but never
 * registered in this fork's ValueContext. AI_VALUE expands to
 * `context->GetValue<T>(name)->Get()`, so every one of those call sites dereferenced a null
 * pointer - the same defect class as "nearest npcs" (see NearestNpcsValue).
 */

#ifndef _PLAYERBOT_FLAGCARRIERVALUE_H
#define _PLAYERBOT_FLAGCARRIERVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

/// The player currently holding a flag in Warsong Gulch / Twin Peaks / Eye of the Storm.
///
/// `sameTeam` picks the bot's own carrier rather than the enemy's; `ignoreRange` returns the
/// carrier wherever they are instead of only within sight range.
class FlagCarrierValue : public UnitCalculatedValue
{
public:
    FlagCarrierValue(PlayerbotAI* botAI, bool sameTeam = false, bool ignoreRange = false)
        : UnitCalculatedValue(botAI), sameTeam(sameTeam), ignoreRange(ignoreRange)
    {
    }

    Unit* Calculate() override;

private:
    bool sameTeam;
    bool ignoreRange;
};

#endif
