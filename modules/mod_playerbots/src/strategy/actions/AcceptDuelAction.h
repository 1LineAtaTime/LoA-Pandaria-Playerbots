#ifndef _PLAYERBOT_ACCEPTDUELACTION_H
#define _PLAYERBOT_ACCEPTDUELACTION_H

#include "Action.h"

class PlayerbotAI;

// AC parity (Ai/Base/Actions/AcceptDuelAction.cpp): answers a pending duel request. 5.4.8 merged
// CMSG_DUEL_ACCEPTED/CMSG_DUEL_CANCELLED into a single CMSG_DUEL_RESPONSE (bool accepted), handled
// by WorldSession::HandleDuelResponseOpcode (DuelHandler.cpp). That handler decides everything from
// Player::duel on the session's own player and never actually uses the guid it reads back, but the
// opponent's guid is written anyway to keep the packet shape honest.
class AcceptDuelAction : public Action
{
public:
    AcceptDuelAction(PlayerbotAI* botAI) : Action(botAI, "accept duel") {}

    bool Execute(Event event) override;

private:
    static void SendDuelResponse(Player* bot, ObjectGuid const& guid, bool accepted);
};

#endif
