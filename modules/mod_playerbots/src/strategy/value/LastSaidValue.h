/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LASTSAIDVALUE_H
#define _PLAYERBOT_LASTSAIDVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

#include <ctime>

class PlayerbotAI;

// Ported from AC mod-playerbots Ai/Base/Value/LastSaidValue.h (registered in AC Ai/Base/ValueContext.h:232-233).
// SayAction ("say::<text>") reads and sets "last said"::<text>. The chat-reply cooldown ("last said"::"chat" in AC)
// is kept by ChatReplyAction itself on this fork, see SayAction.h.
class LastSaidValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastSaidValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, time(nullptr) - 120, "last said") {}
};

class LastEmoteValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastEmoteValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, time(nullptr) - 120, "last emote") {}
};

#endif
