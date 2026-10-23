/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/EnemyHealerTargetValue.h)
 * into the Legends of Azeroth Pandaria fork (issue #149).
 */

#ifndef _PLAYERBOT_ENEMYHEALERTARGETVALUE_H
#define _PLAYERBOT_ENEMYHEALERTARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class EnemyHealerTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    EnemyHealerTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "enemy healer target") {}

protected:
    Unit* Calculate() override;
};

#endif
