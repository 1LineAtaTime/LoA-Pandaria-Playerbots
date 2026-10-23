/*
 * AC parity (#408): ported from AC mod-playerbots Ai/Base/Trigger/PullTriggers.h.
 *
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

/*
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   David Parra Ausina (davidonete/Flekz) <davidparraausina@gmail.com> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 *   Cyberium <cyberium@users.noreply.github.com>
 */

#ifndef _PLAYERBOT_PULLTRIGGERS_H
#define _PLAYERBOT_PULLTRIGGERS_H

#include "Trigger.h"

class PullStartTrigger : public Trigger
{
public:
    PullStartTrigger(PlayerbotAI* botAI, std::string const name = "pull start") : Trigger(botAI, name) {}

    bool IsActive() override;
};

class PullEndTrigger : public Trigger
{
public:
    PullEndTrigger(PlayerbotAI* botAI, std::string const name = "pull end") : Trigger(botAI, name) {}

    bool IsActive() override;
};

class ReturnToPullPositionTrigger : public Trigger
{
public:
    ReturnToPullPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "return to pull position") {}

    bool IsActive() override;
};

#endif
