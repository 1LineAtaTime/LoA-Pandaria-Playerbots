#ifndef _PLAYERBOT_TALKACTION_H
#define _PLAYERBOT_TALKACTION_H

#include "Action.h"

class PlayerbotAI;
class Unit;

// AC parity (Ai/Base/Actions/EmoteAction.cpp:789-874's TalkAction / EmoteAction.h:45-52), split into
// its own file on this fork rather than sharing EmoteAction.cpp/.h. Idle chatter: picks a nearby
// friendly player and sends a small talk/expression text emote at them. Wired onto "often" in
// EmoteStrategy.cpp, gated behind AiPlayerbot.RandomBotEmote - the same AC gate as EmoteAction's
// "receive text emote"/"receive emote" triggers (issue #194, S8).
class TalkAction : public Action
{
public:
    TalkAction(PlayerbotAI* botAI) : Action(botAI, "talk") {}

    bool Execute(Event event) override;

private:
    // AC parity (EmoteActionBase::GetTarget(), EmoteAction.cpp:126-143): a random nearby friendly
    // player within AiPlayerbot.TooCloseDistance.
    Unit* GetTarget();

    // AC parity (TalkAction::GetRandomEmote(unit, true), EmoteAction.cpp:815-846), text-emote branch
    // only - this fork's TalkAction always sends a text emote, same as AC's call site.
    uint32 GetRandomEmote(Unit* unit);
};

#endif
