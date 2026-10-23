/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RepairAllAction.cpp for issue #188 (S6 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - chat->formatMoney(totalCost): ported in #331 as ChatHelper::formatMoney (AC
 *    Bot/Cmd/ChatHelper.cpp:190) and used here since #334. The local copy this file used to carry is gone.
 *  - botAI->HasCheat(BotCheatMask::gold): the whole bot-cheat-mask feature is dormant on this fork
 *    (PlayerbotAI.h:319 has `cheatMask` commented out, no HasCheat()/BotCheatMask exist) and
 *    PlayerbotAI.h is off-limits for this issue. The temporary "give the bot infinite gold so the
 *    repair always succeeds, then restore their real balance" behavior is dropped - repair cost comes
 *    out of the bot's real money like it would for a real player. Flagged as an AC feature that could
 *    not be ported without touching PlayerbotAI.h.
 */

#include "RepairAllAction.h"

#include "Event.h"
#include "ChatHelper.h"
#include "Playerbots.h"

namespace
{
}

bool RepairAllAction::Execute(Event /*event*/)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const guid : npcs)
    {
        Creature* unit = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_REPAIR);
        if (!unit)
            continue;

        if (bot->HasUnitState(UNIT_STATE_DIED))
            bot->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

        bot->SetFacingToObject(unit);
        float discountMod = bot->GetReputationPriceDiscount(unit);

        // Repair weapons first.
        uint32 totalCost = bot->DurabilityRepair(EQUIPMENT_SLOT_MAINHAND, true, discountMod, false);
        totalCost += bot->DurabilityRepair(EQUIPMENT_SLOT_RANGED, true, discountMod, false);
        totalCost += bot->DurabilityRepair(EQUIPMENT_SLOT_OFFHAND, true, discountMod, false);

        totalCost += bot->DurabilityRepairAll(true, discountMod, false);

        if (totalCost > 0)
        {
            std::ostringstream out;
            out << "Repair: " << ChatHelper::formatMoney(totalCost) << " (" << unit->GetName() << ")";
            botAI->TellMasterNoFacing(out.str());

            bot->PlayDistanceSound(1116);
        }

        context->GetValue<uint32>("death count")->Set(0);

        return true;
    }

    botAI->TellError("Cannot find any npc to repair at");
    return false;
}
