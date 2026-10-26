/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ResetAiAction.cpp (#177). Deviations:
 *  - AC's version also runs on an incoming SMSG_GROUP_LIST WorldPacket and bails out early while the
 *    group roster is still non-empty (a race guard for its own dual chat-command/packet-handler
 *    registration). This fork registers "reset botAI" as a chat command only (ChatActionContext.h),
 *    so event.getPacket() is always empty here; the guard is dropped as dead code for that path.
 *  - AC then re-picks a new master with botAI->FindNewMaster() when the bot's random-bot master
 *    mismatch is detected. This fork has no public FindNewMaster() (the equivalent logic is inlined
 *    in PlayerbotAI::UpdateAIGroupMaster(), a .cpp-private flow - see group-functionality-parity.md
 *    section 2 "Master selection"); that flow already re-evaluates the master every tick, so clearing
 *    a stale master here is enough - the next tick's UpdateAIGroupMaster() picks the replacement.
 */

#include "ResetAiAction.h"

#include "Event.h"
#include "Group.h"
#include "PlayerbotRepository.h"
#include "Playerbots.h"

bool ResetAiAction::Execute(Event /*event*/)
{
    if (Player* master = botAI->GetMaster())
    {
        Group* botGroup = bot->GetGroup();
        Group* masterGroup = master->GetGroup();
        if (botGroup && (!masterGroup || masterGroup != botGroup))
            botAI->SetMaster(nullptr);
    }

    PlayerbotRepository::instance().Reset(botAI);
    botAI->ResetStrategies(false);
    botAI->Reset(false);
    botAI->TellMaster("AI was reset to defaults");
    return true;
}
