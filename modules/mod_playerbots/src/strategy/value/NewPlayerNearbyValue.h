/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/NewPlayerNearbyValue.h)
 * into the Legends of Azeroth Pandaria fork (issue #149).
 */

#ifndef _PLAYERBOT_NEWPLAYERNEARBYVALUE_H
#define _PLAYERBOT_NEWPLAYERNEARBYVALUE_H

#include "ObjectGuid.h"
#include "Value.h"

class PlayerbotAI;

class NewPlayerNearbyValue : public CalculatedValue<ObjectGuid>
{
public:
    NewPlayerNearbyValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "new player nearby") {}

    ObjectGuid Calculate() override;
};

// Not one of the issue #149 names, but registered alongside "new player nearby": its Calculate()
// reads "already seen players" (AI_VALUE2(GuidSet&, ...)), so that value must exist too or the
// ported code above hits the exact same null-Value crash this issue is fixing.
class AlreadySeenPlayersValue : public ManualSetValue<GuidSet&>
{
public:
    AlreadySeenPlayersValue(PlayerbotAI* botAI) : ManualSetValue<GuidSet&>(botAI, data, "already seen players") {}

private:
    GuidSet data;
};

#endif
