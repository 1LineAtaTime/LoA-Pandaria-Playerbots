/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Event.h"

#include "Playerbots.h"

Event::Event(std::string const source, ObjectGuid object, Player* owner) : source(source), owner(owner)
{
    packet << object;
}

ObjectGuid Event::getObject()
{
    // An ObjectGuid is written as a raw uint64 (operator<<, ObjectGuid.cpp:106), so getObject() can only
    // decode a packet carrying at least those 8 bytes. An emptiness check alone is not enough:
    // WorldPacketTrigger::Check() binds a real client packet straight into the event (WorldPacketTrigger.cpp:21),
    // and a short-but-non-empty packet then reaches ByteBuffer::read<uint64>() and throws
    // ByteBufferPositionException (ByteBuffer.h:462) out of QuestAction::Execute (QuestAction.cpp:142) on the
    // map update thread - an uncaught exception that terminates the worldserver. Concretely CMSG_GOSSIP_HELLO and
    // CMSG_QUESTGIVER_HELLO carry only a bit-packed guid (~5-6 bytes: one mask byte plus the nonzero index
    // bytes, ByteBuffer.h appendPackGUID) and map to the "gossip hello" trigger (PlayerbotAI.cpp:235-236), which
    // QuestStrategies.cpp:33/48 hands to "talk to quest giver" - a TalkToQuestGiverAction, which does not
    // override Execute and so runs this exact code. Such a packet is not a raw-guid payload at all, so
    // ObjectGuid::Empty is the correct answer and callers keep the fallback they already have for a guid-less
    // event (QuestAction.cpp:150-160, bot/master GetTarget()).
    if (packet.size() < sizeof(uint64))
        return ObjectGuid::Empty;

    WorldPacket p(packet);
    p.rpos(0);

    ObjectGuid guid;
    p >> guid;

    return guid;
}
