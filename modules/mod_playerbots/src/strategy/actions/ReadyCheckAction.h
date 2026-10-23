/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/ReadyCheckAction.h) into the
 * Legends of Azeroth Pandaria fork (#210 S10 G4). See ReadyCheckAction.cpp for the full port notes.
 * #628: AC's ForceRebuffAction ("rebuff" chat command) and ReadyReplyAction (the deferred ready reply of
 * the "force rebuff" strategy, AI/ForceRebuff.cpp) are ported too (AC ReadyCheckAction.h:34-49).
 */

#ifndef _PLAYERBOT_READYCHECKACTION_H
#define _PLAYERBOT_READYCHECKACTION_H

#include "Action.h"

class PlayerbotAI;

class ReadyCheckAction : public Action
{
public:
    ReadyCheckAction(PlayerbotAI* botAI, std::string const name = "ready check") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override { return true; }

protected:
    bool ReadyCheck();
    void ReportReadinessToMaster();
};

class FinishReadyCheckAction : public ReadyCheckAction
{
public:
    FinishReadyCheckAction(PlayerbotAI* botAI) : ReadyCheckAction(botAI, "finish ready check") {}

    bool Execute(Event event) override;
};

class ForceRebuffAction : public Action
{
public:
    ForceRebuffAction(PlayerbotAI* botAI) : Action(botAI, "force rebuff") {}

    bool Execute(Event event) override;
};

class ReadyReplyAction : public Action
{
public:
    ReadyReplyAction(PlayerbotAI* botAI) : Action(botAI, "ready reply") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
