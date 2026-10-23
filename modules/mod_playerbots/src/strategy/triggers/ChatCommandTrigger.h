/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported verbatim from AC mod-playerbots Ai/Base/Trigger/ChatCommandTrigger.h (#173). A generic
 * "someone typed this chat command" trigger: ExternalEventHelper::HandleCommand() arms it with the
 * command's trailing text and the whisperer/party-member who sent it; the next Engine tick's
 * Check() then fires the matching chat Action once (Reset() disarms it again).
 */

#ifndef _PLAYERBOT_CHATCOMMANDTRIGGER_H
#define _PLAYERBOT_CHATCOMMANDTRIGGER_H

#include "Trigger.h"

class Event;
class Player;
class PlayerbotAI;

class ChatCommandTrigger : public Trigger
{
public:
    ChatCommandTrigger(PlayerbotAI* botAI, std::string const command);

    using Trigger::ExternalEvent;
    void ExternalEvent(std::string const param, Player* owner = nullptr) override;
    Event Check() override;
    void Reset() override;

private:
    std::string param;
    bool triggered;
    Player* owner;
};

#endif
