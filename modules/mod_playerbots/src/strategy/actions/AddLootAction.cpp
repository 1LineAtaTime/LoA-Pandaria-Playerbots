/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/AddLootAction.cpp (issue #151), unchanged - no
 * 5.4.8 API differences apply here.
 */

#include "AddLootAction.h"

#include "Event.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

bool AddLootAction::Execute(Event event)
{
    // AC AddLootAction.cpp:11-18 reads the plain guid of the master's CMSG_GAMEOBJ_USE (#535 wires
    // "use game object" -> "add loot", AC WorldPacketHandlerStrategy.cpp:29-30). This core bit-packs that
    // guid (Handlers/SpellHandler.cpp HandleGameObjectUseOpcode), so decode it the same way
    // UseMeetingStoneAction::Execute does; a raw read would yield a garbage guid.
    ObjectGuid guid;
    if (event.GetSource() == "use game object" && !event.getPacket().empty())
    {
        WorldPacket p(event.getPacket());
        p.rpos(0);
        guid[6] = p.ReadBit();
        guid[1] = p.ReadBit();
        guid[3] = p.ReadBit();
        guid[4] = p.ReadBit();
        guid[0] = p.ReadBit();
        guid[5] = p.ReadBit();
        guid[7] = p.ReadBit();
        guid[2] = p.ReadBit();

        p.ReadByteSeq(guid[0]);
        p.ReadByteSeq(guid[1]);
        p.ReadByteSeq(guid[6]);
        p.ReadByteSeq(guid[2]);
        p.ReadByteSeq(guid[3]);
        p.ReadByteSeq(guid[4]);
        p.ReadByteSeq(guid[5]);
        p.ReadByteSeq(guid[7]);
    }
    else
        guid = event.getObject();

    if (!guid)
        return false;

    return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
}

bool AddAllLootAction::Execute(Event /*event*/)
{
    bool added = false;

    GuidVector gos = context->GetValue<GuidVector>("nearest game objects")->Get();
    for (GuidVector::iterator i = gos.begin(); i != gos.end(); i++)
        added |= AddLoot(*i);

    GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    for (GuidVector::iterator i = corpses.begin(); i != corpses.end(); i++)
        added |= AddLoot(*i);

    return added;
}

bool AddLootAction::isUseful() { return true; }

bool AddAllLootAction::isUseful() { return true; }

bool AddAllLootAction::AddLoot(ObjectGuid guid) { return AI_VALUE(LootObjectStack*, "available loot")->Add(guid); }

bool AddGatheringLootAction::AddLoot(ObjectGuid guid)
{
    LootObject loot(bot, guid);

    WorldObject* wo = loot.GetWorldObject(bot);
    if (loot.IsEmpty() || !wo)
        return false;

    if (loot.skillId == SKILL_NONE)
        return false;

    if (!loot.IsLootPossible(bot))
        return false;

    return AddAllLootAction::AddLoot(guid);
}
