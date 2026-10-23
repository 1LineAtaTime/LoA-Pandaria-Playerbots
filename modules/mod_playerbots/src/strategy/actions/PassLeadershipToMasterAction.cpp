/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/PassLeadershipToMasterAction.cpp (#177). Deviations:
 *  - (#650, MapUpdate.Threads > 1) The leader change is marshalled onto the world thread exactly as AC does
 *    (AC PassLeadershipToMasterAction.cpp:17-18): a GroupSetLeaderOperation queued on the ported
 *    PlayerbotWorldThreadProcessor. Bot actions run in Player::Update on the map-update threads, and the group
 *    is shared with members on other maps. (The earlier note that every action runs on the world thread was
 *    only true with a single map-update thread.)
 *  - PassLeadershipToMasterAction::isUseful() (the automatic, unprompted self-trigger AC uses so an
 *    "alt bot" hands leadership back to its owning player without being asked) relied on
 *    botAI->IsAltBot(), a concept this fork's PlayerbotAI does not have. Rather than guess at an
 *    equivalent, the base class's isUseful() is left returning false (never auto-fires); only the
 *    explicit "give leader" chat command (GiveLeaderAction, its own isUseful() below) is wired up.
 *  - GiveLeaderAction::isUseful() uses this fork's own vocabulary: "IsRealPlayer(Player*)" (AC, a
 *    live human client) becomes "master has no PlayerbotAI"; "IsSelfBot(Player*)" becomes this
 *    fork's PlayerbotAI::IsRealPlayer() (self-bot: master == bot - yes, the two forks use the same
 *    two words for opposite concepts; see the AC-port-ground-truth notes).
 */

#include "PassLeadershipToMasterAction.h"

#include "Event.h"
#include "Group.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"

bool PassLeadershipToMasterAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master || master == bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group || !group->IsMember(master->GetGUID()))
        return false;

    // AC PassLeadershipToMasterAction.cpp:17-18: queue the leader change for the world thread.
    auto setLeaderOp = std::make_unique<GroupSetLeaderOperation>(bot->GetGUID(), master->GetGUID());
    PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(setLeaderOp));

    if (!message.empty())
        botAI->TellMaster(message);

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        botAI->ResetStrategies();
        botAI->Reset();
    }

    return true;
}

// Never fires on its own - see the class comment above. GiveLeaderAction (the explicit chat command)
// has its own isUseful() below.
bool PassLeadershipToMasterAction::isUseful() { return false; }

bool GiveLeaderAction::isUseful()
{
    Player* master = botAI->GetMaster();
    bool const masterIsHuman = master && !GET_PLAYERBOT_AI(master);
    bool const selfBot = botAI->IsRealPlayer();
    return (masterIsHuman || selfBot) && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetGUID());
}
