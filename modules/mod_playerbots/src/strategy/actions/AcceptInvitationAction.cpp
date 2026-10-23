#include "AcceptInvitationAction.h"

#include "Event.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "WorldPacket.h"

AcceptInvitationAction::AcceptInvitationAction(PlayerbotAI* botAI, const std::string name)
    : SummonAction(botAI, name)
{
}

bool AcceptInvitationAction::Execute(Event event)
{
    Group* grp = bot->GetGroupInvite();
    if (!grp)
        return false;

    // #161 AC parity (AC AcceptInvitationAction.cpp:26): the inviter is the pending invite's group leader.
    // The old code parsed the 5.4.8 invite packet with a hard-coded read_skip(27) and silently refused the
    // invite whenever the name came out wrong.
    Player* inviter = ObjectAccessor::FindPlayer(grp->GetLeaderGUID());
    if (!inviter)
        return false;

    if (!botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, inviter))
    {
        WorldPacket data(SMSG_GROUP_DECLINE, 10);
        data << bot->GetName();
        inviter->SendDirectMessage(&data);
        bot->UninviteFromGroup();
        return false;
    }

    if (bot->isAFK())
        bot->ToggleAFK();

    WorldPacket p;
    uint8 unk = 0;
    p << unk;
    p.WriteBit(true);
    p.WriteBit(true);
    p.FlushBits();
    bot->GetSession()->HandleGroupInviteResponseOpcode(p);

    if (!bot->GetGroup() || !bot->GetGroup()->IsMember(inviter->GetGUID()))
        return false;

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
        botAI->SetMaster(inviter);

    botAI->ResetStrategies();
    botAI->ChangeStrategy("+follow,-lfg,-bg", BOT_STATE_NON_COMBAT);
    botAI->Reset();

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "hello", "Hello", {}));

    // AC AcceptInvitationAction.cpp:61-64 (#177): summon the bot to the inviter if it accepted from
    // out of sight range.
    if (sPlayerbotAIConfig->summonWhenGroup && bot->GetDistance(inviter) > sPlayerbotAIConfig->sightDistance)
    {
        Teleport(inviter, bot, true);
    }
    return true;
}
