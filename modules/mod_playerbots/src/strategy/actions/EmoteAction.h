#ifndef _PLAYERBOT_EMOTEACTION_H
#define _PLAYERBOT_EMOTEACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;
class Unit;

// AC parity: EmoteActionBase::Emote() / PlayerbotAI::PlayEmote() (AC Bot/PlayerbotAI.cpp:1886-1893)
// send a real text emote through the core's own CMSG_TEXT_EMOTE handler, instead of an animation plus
// a plain Say(). Ported as a free helper (see PetsAction.h's PlayerbotPetFollow for the same pattern:
// AI/PlayerbotAI.h is not touched) so EmoteAction, TalkAction and GreetAction can all share it.
void PlayBotTextEmote(Player* bot, uint32 textEmoteId, Unit* target = nullptr);

// AC parity (Ai/Base/Actions/EmoteAction.{h,cpp} - EmoteActionBase/EmoteAction). ReceiveEmote() is a
// full, not scoped-down, port of AC's ~150-case reaction table (issue #194, S8; the previous pass's
// "representative subset" scoping is completed here). The receiving-side packet parsing (SMSG_TEXT_EMOTE
// / SMSG_EMOTE) is the 5.4.8 layout from s8-pet-control-and-class-spells.md Part B.9 and is unchanged
// from the previous pass.
class EmoteAction : public Action
{
public:
    EmoteAction(PlayerbotAI* botAI) : Action(botAI, "emote") {}

    bool Execute(Event event) override;

private:
    // AC parity (EmoteAction.cpp:145-647): react to an incoming text emote from `source`. `verbal`
    // gates the Say()/Yell() flavor line (AC never says/yells while the bot is in a battleground).
    bool ReceiveEmote(Player* source, uint32 emote, bool verbal);
};

#endif
