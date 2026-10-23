/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/LfgValues.h) into the
 * Legends of Azeroth Pandaria fork (issue #149). This fork's dungeon-finder proposal handling
 * (AI/PlayerbotAI.cpp) currently only comments out its use of this value; registering it now
 * means that call site (and any LFG code that reactivates it) won't dereference a null Value.
 */

#ifndef _PLAYERBOT_LFGVALUES_H
#define _PLAYERBOT_LFGVALUES_H

#include "Value.h"

class PlayerbotAI;

class LfgProposalValue : public ManualSetValue<uint32>
{
public:
    LfgProposalValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "lfg proposal") {}
};

#endif
