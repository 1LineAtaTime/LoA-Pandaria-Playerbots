/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/AreaTriggerAction.cpp (#179). Deviations from AC:
 *  - this core's CMSG_AREATRIGGER (Handlers/MiscHandler.cpp:HandleAreaTriggerOpcode, verified) carries
 *    two extra bits AC's WotLK opcode does not: `fromClient` and `entered` - the same opcode fires on
 *    both entering AND leaving a trigger zone. ReachAreaTriggerAction ignores a `entered=0` (leaving)
 *    packet; AC has nothing to ignore here because its wire format has no such bit.
 *  - `AreaTrigger const*` (AC, a DBC-only struct with map/x/y/z/radius/box) does not exist on this
 *    core: the geometry comes from `sAreaTriggerStore` (AreaTriggerEntry, DBCStructure.h, verified)
 *    and the teleport-target lookup that AC does with `GetAreaTriggerTeleport()` is this core's
 *    `ObjectMgr::GetAreaTrigger()` (ObjectMgr.h, verified) - it returns non-null only for a trigger
 *    that has a configured teleport destination, i.e. the same "is this a portal" test.
 *  - `IsSelfBot(bot)` -> `botAI->IsRealPlayer()` (established fork mapping).
 *  - `MotionMaster::MovePoint` takes this core's 5-arg form (id, x, y, z, generatePath), not AC's
 *    8-arg one; `true` keeps AC's "terrain path (2d mmap)" intent for this call.
 *  - no PlayerbotTextMgr: plain default strings, matching AC's own fallback text.
 *  - the bot's own re-fired CMSG_AREATRIGGER packet is bit-packed for `fromClient`/`entered`
 *    (WriteBit/FlushBits) to match this core's wire format; AC's WotLK packet is a bare uint32.
 */

#include "AreaTriggerAction.h"

#include "DBCStores.h"
#include "DBCStructure.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "WorldSession.h"

bool ReachAreaTriggerAction::Execute(Event event)
{
    // AC: "Selfbots reach their own area triggers through the client."
    if (botAI->IsRealPlayer())
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint32 triggerId;
    p >> triggerId;
    bool const fromClient = p.ReadBit();
    bool const entered = p.ReadBit();
    (void)fromClient;

    // 5.4.8 adaptation (see file comment): ignore the master *leaving* a trigger zone.
    if (!entered)
        return false;

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(triggerId);
    if (!atEntry)
        return false;

    if (!sObjectMgr->GetAreaTrigger(triggerId))
    {
        // Not a teleporting trigger (e.g. a scripted/quest area trigger): fire the bot's own use of
        // it immediately, exactly like AC does for a non-teleport GetAreaTriggerTeleport() miss.
        WorldPacket p1(CMSG_AREATRIGGER);
        p1 << uint32(triggerId);
        p1.WriteBit(0);  // fromClient
        p1.WriteBit(1);  // entered
        p1.FlushBits();
        p1.rpos(0);
        bot->GetSession()->HandleAreaTriggerOpcode(p1);

        return true;
    }

    if (bot->GetMapId() != atEntry->ContinentID)
    {
        botAI->TellError("I won't follow: too far away");
        return true;
    }

    bot->GetMotionMaster()->MovePoint(atEntry->ID, atEntry->Pos.X, atEntry->Pos.Y, atEntry->Pos.Z, true);

    float distance = bot->GetDistance(atEntry->Pos.X, atEntry->Pos.Y, atEntry->Pos.Z);
    float delay = 1000.0f * distance / bot->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;
    botAI->TellError("Wait for me");
    botAI->SetNextCheckDelay((uint32)delay);
    AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = triggerId;

    return true;
}

bool AreaTriggerAction::Execute(Event /*event*/)
{
    LastMovement& movement = AI_VALUE(LastMovement&, "last area trigger");

    uint32 triggerId = movement.lastAreaTrigger;
    movement.lastAreaTrigger = 0;

    if (!sAreaTriggerStore.LookupEntry(triggerId))
        return false;

    if (!sObjectMgr->GetAreaTrigger(triggerId))
        return true;

    WorldPacket p(CMSG_AREATRIGGER);
    p << uint32(triggerId);
    p.WriteBit(0);  // fromClient
    p.WriteBit(1);  // entered
    p.FlushBits();
    p.rpos(0);
    bot->GetSession()->HandleAreaTriggerOpcode(p);

    botAI->TellMaster("Hello");
    return true;
}
