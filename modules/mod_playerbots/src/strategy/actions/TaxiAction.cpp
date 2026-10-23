/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TaxiAction.cpp (#179). Deviations from AC:
 *  - no PlayerbotTextMgr on this fork (same pattern as every other ported chat/tell string): plain
 *    default strings are used directly, matching AC's own fallback text.
 *  - AC's botAI->AddTimedEvent(lambda, delay) has no equivalent on this fork's PlayerbotAI; the
 *    core already gives every Unit a scheduler (Unit::Schedule(TimeValue, function),
 *    src/server/game/Entities/Unit/Unit.h:2107, backed by EventProcessor - the same mechanism the
 *    core itself uses for delayed unit events), so the staggered take-off is scheduled on
 *    bot->Schedule(...) instead. The lambda re-resolves both the bot and the NPC by guid instead of
 *    capturing raw pointers/references, since nothing guarantees either is still valid seconds later.
 *  - botAI->HasGameClientMaster() -> botAI->HasRealPlayerMaster() (established fork mapping, see
 *    e.g. AiFactory.cpp's "avoid aoe" gate).
 *  - the "taxi" chat command is wired since #767 (ChatActionContext.h / ChatTriggerContext.h /
 *    ChatCommandHandlerStrategy.cpp). As on AC, a bot with a real-player master always takes the
 *    follower branch below and returns true even with no remembered nodes (#767: an early
 *    `return false` on empty taxiNodes made every master-whispered "taxi"/"taxi ?" log FAILED where
 *    AC logs OK); the scheduled lambda is what checks the nodes, exactly like AC.
 *  - AC's `uint8((i - 1) / 32) < TaxiMaskSize` filter on the path id is not ported: it assumes AC's
 *    14 x uint32 mask (path ids <= 448), while 5.4.8 TaxiPath.dbc ids run to 3924 (Ironforge alone
 *    has paths 2360/2366/2522/2531), so it would drop real Cata/MoP routes from the "?" list.
 *  - the numeric selection keeps a bounds check (AC indexes nodes[selected - 1] unchecked).
 */

#include "TaxiAction.h"

#include <chrono>

#include "DBCStores.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "TimeValue.h"
#include "WorldSession.h"

bool TaxiAction::Execute(Event event)
{
    botAI->RemoveShapeshift();

    LastMovement& movement = AI_VALUE(LastMovement&, "last taxi");

    WorldPacket& p = event.getPacket();
    std::string const param = event.getParam();
    // 5.4.8 has no CMSG_TAXICLEARALLNODES/CMSG_TAXICLEARNODE (see the opcode-mapping research); the
    // "clear" param path below (unreachable until the chat layer wires "taxi" - see the file comment)
    // is kept for AC parity.
    if (param == "clear")
    {
        movement.taxiNodes.clear();
        movement.Set(nullptr);
        botAI->TellMaster("I am ready for the next flight");
        return true;
    }

    GuidVector units = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const guid : units)
    {
        Creature* npc = ObjectAccessor::GetCreature(*bot, guid);
        if (!npc || !npc->IsAlive())
            continue;

        if (!npc->IsTaxi())
            continue;

        if (bot->GetDistance(npc) > sPlayerbotAIConfig->farDistance)
            continue;

        // AC TaxiAction.cpp:46-60: learn this flight master's node, then list the paths leaving it.
        bot->GetSession()->SendLearnNewTaxiNode(npc);
        uint32 curloc = sObjectMgr->GetNearestTaxiNode(npc->GetPositionX(), npc->GetPositionY(),
            npc->GetPositionZ(), npc->GetMapId(), bot->GetTeam());

        std::vector<uint32> nodes;
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
            if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
                if (entry->from == curloc)
                    nodes.push_back(i);

        // Only for follower bots: stagger the take-off so a group of bots does not all leave the
        // flight master on the same tick (spec section 3, taxi-boarding-delay.md). AC returns true
        // here even with no remembered nodes; the lambda checks them (AC TaxiAction.cpp:63-86).
        if (botAI->HasRealPlayerMaster())
        {
            uint32 index = botAI->GetGroupSlotIndex(bot);
            uint32 delay = sPlayerbotAIConfig->botTaxiDelayMin +
                          index * sPlayerbotAIConfig->botTaxiDelayGapMs +
                          urand(0, sPlayerbotAIConfig->botTaxiDelayGapJitterMs);

            delay = std::min(delay, sPlayerbotAIConfig->botTaxiDelayMax);

            ObjectGuid const botGuid = bot->GetGUID();
            ObjectGuid const npcGuid = npc->GetGUID();
            std::vector<uint32> const nodes = movement.taxiNodes;

            bot->Schedule(TimeValue(std::chrono::milliseconds(delay)), [botGuid, npcGuid, nodes]()
            {
                Player* self = ObjectAccessor::FindPlayer(botGuid);
                if (!self || !self->IsInWorld())
                    return;

                if (Creature* npcPtr = ObjectAccessor::GetCreature(*self, npcGuid))
                    if (!nodes.empty())
                        self->ActivateTaxiPathTo(nodes, npcPtr, 0);
            });

            botAI->SetNextCheckDelay(delay + 50);
            return true;
        }

        if (param == "?")
        {
            botAI->TellMasterNoFacing("=== Taxi ===");

            uint32 index = 1;
            for (uint32 node : nodes)
            {
                TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(node);
                if (!entry)
                    continue;

                TaxiNodesEntry const* dest = sTaxiNodesStore.LookupEntry(entry->to);
                if (!dest)
                    continue;

                std::ostringstream out;
                out << index++ << ": " << dest->name;
                botAI->TellMasterNoFacing(out.str());
            }

            return true;
        }

        uint32 selected = atoi(param.c_str());
        if (selected)
        {
            if (selected > nodes.size())
                return false;

            uint32 path = nodes[selected - 1];
            TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
            if (!entry)
                return false;

            return bot->ActivateTaxiPathTo({entry->from, entry->to}, npc, 0);
        }

        if (!movement.taxiNodes.empty() && !bot->ActivateTaxiPathTo(movement.taxiNodes, npc, 0))
        {
            movement.taxiNodes.clear();
            movement.Set(nullptr);
            botAI->TellError("I can't fly with you");
            return false;
        }

        return true;
    }

    botAI->TellError("Cannot find any flightmaster to talk");
    return false;
}
