/*
 * Ported from AC mod-playerbots Ai/Base/Actions/GreetAction.cpp (issue #194, S8).
 *
 * Deviations from AC:
 *  - botAI->PlayEmote(TEXT_EMOTE_HELLO) (a PlayerbotAI member on AC) is replaced by the free helper
 *    PlayBotTextEmote() (EmoteAction.h), so AI/PlayerbotAI.h does not need a new member for this.
 *  - NewPlayerNearbyValue/AlreadySeenPlayersValue and the "new player nearby" trigger already existed
 *    on this fork (issue #149) but the trigger was never registered in TriggerContext.h - fixed
 *    alongside this file (see TriggerContext.h's #194 comment).
 */

#include "GreetAction.h"

#include "EmoteAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "BotArc.h"

bool GreetAction::Execute(Event /*event*/)
{
    ObjectGuid guid = AI_VALUE(ObjectGuid, "new player nearby");
    if (!guid || !guid.IsPlayer())
        return false;

    Player* player = dynamic_cast<Player*>(botAI->GetUnit(guid));
    if (!player)
        return false;

    if (!BotHasInArcAC(bot, CAST_ANGLE_IN_FRONT, player, sPlayerbotAIConfig->sightDistance))
        bot->SetFacingToObject(player);

    ObjectGuid oldSel = bot->GetTarget();
    bot->SetTarget(guid);
    PlayBotTextEmote(bot, TEXT_EMOTE_HELLO, player);
    bot->SetTarget(oldSel);

    GuidSet& alreadySeenPlayers = AI_VALUE(GuidSet&, "already seen players");
    alreadySeenPlayers.insert(guid);

    GuidVector nearestPlayers = AI_VALUE(GuidVector, "nearest friendly players");
    for (ObjectGuid const& nearGuid : nearestPlayers)
        alreadySeenPlayers.insert(nearGuid);

    return true;
}
