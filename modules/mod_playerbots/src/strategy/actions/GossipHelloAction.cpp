/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/GossipHelloAction.cpp for issue #258 slice "Trainer +
 * GossipHello" (the NPC interaction layer): the path that makes a bot open a gossip menu on a creature in
 * the world and answer it.
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *
 *  - Packet layout. AC builds both packets flat (`p << guid;` then `p << menuId << menuToSelect; p << code;`).
 *    On 5.4.8 both CMSG_GOSSIP_HELLO and CMSG_GOSSIP_SELECT_OPTION bit-pack the guid with a per-handler
 *    scrambled field order, so AC's layout is simply unreadable. The two helpers below (ReadGossipHelloGuid
 *    / WriteGossipHelloGuid / WriteGossipSelectGuid) mirror the CORE READERS field for field:
 *      - WorldSession::HandleGossipHelloOpcode, src/server/game/Handlers/NPCHandler.cpp:447-468
 *      - WorldSession::HandleGossipSelectOptionOpcode, src/server/game/Handlers/MiscHandler.cpp:101-127
 *    Writing is done with the same ByteBuffer primitives the reader uses (WriteBit / WriteBits /
 *    WriteByteSeq, ByteBuffer.h:169-214), so the writer order IS the reader order.
 *
 *  - CMSG_GOSSIP_SELECT_OPTION field order is SWAPPED against AC. The core reads
 *    `recvData >> gossipListId >> menuId;` (MiscHandler.cpp:105), i.e. the *menu item id first*, then the
 *    menu id. AC writes menuId first. Followed the core.
 *
 *  - The `code` string is only present on the wire when the selected item is coded. The core reads it
 *    conditionally: `if (_player->PlayerTalkClass->IsGossipOptionCoded(gossipListId)) code =
 *    recvData.ReadString(boxTextLength);` (MiscHandler.cpp:124-125). AC writes it unconditionally. The 8-bit
 *    boxTextLength is always written (the core reads it unconditionally), the payload only when coded.
 *
 *  - GossipMenu / GossipMenuItem / GossipMenuItemContainer / PlayerMenu::GetGossipMenu() /
 *    GetGossipMenuId(menuId, source) all exist on this core with AC's names
 *    (Entities/Creature/GossipDef.h:201-278, :321-329; Entities/Player/Player.h:1689), so that half of the
 *    port is unchanged.
 *
 *  - `GossipText` / `GossipTextOption` / `MAX_GOSSIP_TEXT_OPTIONS` live in src/server/game/Handlers/
 *    NPCHandler.h:25-45 on this core (not in GossipDef.h). That directory is on the module's include path
 *    (build/modules/modules.vcxproj AdditionalIncludeDirectories), so the include resolves.
 *
 *  - `sObjectMgr->GetGossipText()` (Globals/ObjectMgr.h:1129) and
 *    `sObjectMgr->GetGossipMenuItemsMapBounds()` (Globals/ObjectMgr.h:1691) exist unchanged.
 *
 *  - ObjectGuid is used instead of AC's `p >> guid` shorthand. `ObjectGuid` is a `class`
 *    (Entities/Object/ObjectGuid.h:135), so the forward declaration in the header matches it.
 *
 *  - `LOG_DEBUG` -> `TC_LOG_DEBUG` (this fork's log macro).
 *
 *  - BroadcastHelper: not ported on this fork (issue #322). AC's GossipHelloAction makes no BroadcastHelper
 *    call; nothing omitted here.
 *
 *  - PlayerbotTextMgr: not ported on this fork (issue #322). AC's GossipHelloAction reports no failure
 *    through it either - its two user-visible strings ("Unknown gossip option", "I need to talk first") are
 *    the module's own literals, reproduced verbatim below.
 *
 *  - Null-master guards added at the two sites where AC dereferences GetMaster() unguarded
 *    (AC GossipHelloAction.cpp:62 in TellGossipMenus, :92/:95 in ProcessGossip). A masterless bot (a random
 *    bot, or an account bot) would otherwise take the worldserver down; both sites already have a
 *    fail-closed path right next to them, so the guard returns there instead.
 */

#include "GossipHelloAction.h"

// #258: `Player.h` must precede `GossipDef.h` - GossipDef.h declares
// `SendQuestGiverStatus(QuestGiverStatus, ...)` at :338 and that enum is only forward-declared in
// `AI/CreatureAI.h:35`, which Player.h pulls in. Including GossipDef.h first fails with
// "syntax error: identifier 'QuestGiverStatus'" / "identifier 'Quest'" (GossipDef.h:338-346).
#include "Player.h"

#include "Event.h"
#include "GossipDef.h"
#include "NPCHandler.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Playerbots.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <cstdlib>
#include <sstream>

namespace
{
// AC GossipHelloAction.cpp:25-26 `p.rpos(0); p >> guid;`, replaced by the core's own bit layout. Read order is
// WorldSession::HandleGossipHelloOpcode, NPCHandler.cpp:451-468.
ObjectGuid ReadGossipHelloGuid(WorldPacket& p)
{
    ObjectGuid guid;
    guid[2] = p.ReadBit();
    guid[4] = p.ReadBit();
    guid[0] = p.ReadBit();
    guid[3] = p.ReadBit();
    guid[6] = p.ReadBit();
    guid[7] = p.ReadBit();
    guid[5] = p.ReadBit();
    guid[1] = p.ReadBit();

    p.ReadByteSeq(guid[4]);
    p.ReadByteSeq(guid[7]);
    p.ReadByteSeq(guid[1]);
    p.ReadByteSeq(guid[0]);
    p.ReadByteSeq(guid[5]);
    p.ReadByteSeq(guid[3]);
    p.ReadByteSeq(guid[6]);
    p.ReadByteSeq(guid[2]);

    return guid;
}

// Mirror of ReadGossipHelloGuid. AC GossipHelloAction.cpp:125-127 `p1 << guid;`.
void WriteGossipHelloGuid(WorldPacket& data, ObjectGuid const& guid)
{
    data.WriteBit(guid[2]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[1]);

    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
}

// Mirror of WorldSession::HandleGossipSelectOptionOpcode, MiscHandler.cpp:113-134. `boxTextLength` is the
// 8-bit length written between the guid mask and the guid bytes; the caller writes the payload itself,
// between the byte run and the two trailing bytes, and only when the option is coded.
void WriteGossipSelectGuid(WorldPacket& data, ObjectGuid const& guid, size_t boxTextLength)
{
    data.WriteBit(guid[3]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[6]);
    data.WriteBits(boxTextLength, 8);
    data.WriteBit(guid[2]);

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[5]);
}
}  // namespace

bool GossipHelloAction::Execute(Event event)
{
    ObjectGuid guid;

    WorldPacket& p = event.getPacket();
    if (p.empty())
    {
        Player* master = GetMaster();
        if (master)
            guid = master->GetTarget();
    }
    else
    {
        p.rpos(0);
        guid = ReadGossipHelloGuid(p);
    }

    std::string const text = event.getParam();
    int32 menuToSelect = -1;
    if (!text.empty())
        menuToSelect = int32(atoi(text.c_str()));

    return Execute(guid, menuToSelect, false);
}

void GossipHelloAction::TellGossipText(uint32 textId)
{
    if (!textId)
        return;

    if (GossipText const* text = sObjectMgr->GetGossipText(textId))
    {
        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; i++)
        {
            std::string const text0 = text->Options[i].Text_0;
            if (!text0.empty())
                botAI->TellMasterNoFacing(text0);

            std::string const text1 = text->Options[i].Text_1;
            if (!text1.empty())
                botAI->TellMasterNoFacing(text1);
        }
    }
}

void GossipHelloAction::TellGossipMenus()
{
    if (!bot->PlayerTalkClass)
        return;

    // AC GossipHelloAction.cpp:62 dereferences GetMaster() unguarded; see the file header.
    Player* master = GetMaster();
    if (!master)
        return;

    Creature* pCreature = bot->GetNPCIfCanInteractWith(master->GetTarget(), UNIT_NPC_FLAG_NONE);
    GossipMenu& menu = bot->PlayerTalkClass->GetGossipMenu();
    if (pCreature)
    {
        uint32 textId = bot->GetGossipTextId(menu.GetMenuId(), pCreature);
        TellGossipText(textId);
    }

    GossipMenuItemContainer const& items = menu.GetMenuItems();
    for (auto iter = items.begin(); iter != items.end(); iter++)
    {
        GossipMenuItem const* item = &(iter->second);
        std::ostringstream out;
        out << "[" << iter->first << "] " << item->Message;
        botAI->TellMasterNoFacing(out.str());
    }
}

bool GossipHelloAction::ProcessGossip(int32 menuToSelect, bool silent)
{
    if (!bot->PlayerTalkClass)
        return false;

    // AC GossipHelloAction.cpp:92 dereferences GetMaster() unguarded; see the file header.
    Player* master = GetMaster();
    if (!master)
        return false;

    GossipMenu& menu = bot->PlayerTalkClass->GetGossipMenu();
    if (menuToSelect != -1 && !menu.GetItem(uint32(menuToSelect)))
    {
        if (!silent)
            botAI->TellError("Unknown gossip option");
        return false;
    }

    // 5.4.8 layout - see the file header and MiscHandler.cpp:101-134.
    WorldPacket data(CMSG_GOSSIP_SELECT_OPTION, 8 + 1 + 4 + 4);
    std::string code;
    data << uint32(menuToSelect);  // gossipListId: the MENU ITEM id, read first by the core
    data << menu.GetMenuId();     // menuId: read second

    bool coded = bot->PlayerTalkClass->IsGossipOptionCoded(uint32(menuToSelect));
    WriteGossipSelectGuid(data, master->GetTarget(), coded ? code.size() : 0);

    if (coded)
        data.WriteString(code);

    // The core reads the last two guid bytes AFTER the coded string (MiscHandler.cpp:127-128).
    ObjectGuid guid = master->GetTarget();
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[1]);

    bot->GetSession()->HandleGossipSelectOptionOpcode(data);

    if (!silent)
        TellGossipMenus();

    return true;
}

bool GossipHelloAction::Execute(ObjectGuid guid, int32 menuToSelect, bool silent)
{
    if (!guid)
        return false;

    Creature* pCreature = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!pCreature)
    {
        TC_LOG_DEBUG("playerbots",
                     "[PlayerbotMgr]: HandleMasterIncomingPacket - Received  CMSG_GOSSIP_HELLO {} not found or you "
                     "can't interact with him.",
                     guid.ToString().c_str());
        return false;
    }

    GossipMenuItemsMapBounds pMenuItemBounds =
        sObjectMgr->GetGossipMenuItemsMapBounds(pCreature->GetCreatureTemplate()->GossipMenuId);
    if (pMenuItemBounds.first == pMenuItemBounds.second)
        return false;

    if (menuToSelect == -1)
    {
        WorldPacket p1(CMSG_GOSSIP_HELLO, 8 + 1);
        WriteGossipHelloGuid(p1, guid);
        bot->GetSession()->HandleGossipHelloOpcode(p1);
        bot->SetFacingToObject(pCreature);

        if (!silent)
        {
            std::ostringstream out;
            out << "--- " << pCreature->GetName() << " ---";
            botAI->TellMasterNoFacing(out.str());
            TellGossipMenus();
        }
    }
    else if (!bot->PlayerTalkClass)
    {
        if (!silent)
            botAI->TellError("I need to talk first");
        return false;
    }
    else
    {
        if (!ProcessGossip(menuToSelect, silent))
            return false;
    }

    bot->TalkedToCreature(pCreature->GetEntry(), pCreature->GetGUID());
    return true;
}