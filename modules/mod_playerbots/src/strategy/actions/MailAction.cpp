/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/MailAction.cpp (#258). Divergences, all forced by the
 * 5.4.8 core or by this fork's already-ported helpers:
 *  - chat->FormatItem -> ChatHelper::FormatItem (static on this fork).
 *  - ChatHelper::formatMoney() (#331, replacing the file-local FormatMoney() this file carried): the
 *    ChatHelper member landed in AI/ChatHelper.h. That copy was byte-identical to AC's
 *    (AC Bot/Cmd/ChatHelper.cpp:190-229), so the money shown in the mail output does not change.
 *  - Player::GetMails() does not exist on 5.4.8; AC's loop over GetMails() walks
 *    Player::GetMailBegin()/GetMailEnd() (core Player.h:1983-1995).
 *  - The three client packets AC feeds to the WorldSession mail handlers must be built in the order
 *    the 5.4.8 core READS them, not AC's 3.3.5 order. From
 *    src/server/game/Handlers/MailHandler.cpp:
 *      HandleMailDelete    (:459-464) reads mailId, then skips mailTemplateId. No guid at all.
 *      HandleMailTakeItem  (:571-595) reads mailId, itemId, then a packed mailbox guid.
 *      HandleMailTakeMoney (:689-714) reads mailId, money (uint64), then a packed mailbox guid.
 *    AC writes (mailbox lowguid, mailId[, itemId]). Using that order here makes
 *    WorldSession::CanOpenMailBox() (MailHandler.cpp:38-62) reject the bot, so the mailbox is packed
 *    by hand below in exactly the order those three readers consume it.
 */

#include "MailAction.h"

#include "AiObjectContext.h"
#include "ChatHelper.h"
#include "Event.h"
#include "GameObject.h"
#include "Helper.h"
#include "Item.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "WorldSession.h"

#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Writes `mailbox` in the packed-guid layout the 5.4.8 mail handlers read, not the raw 8-byte layout
// operator<<(ByteBuffer&, ObjectGuid const&) produces (core ObjectGuid.cpp) and not AC's 3.3.5
// lowguid.
//
// ByteBuffer::ReadBit() (core ByteBuffer.h:198-208) consumes the mask byte MSB-first, so the reader's
// n-th ReadBit() is mask bit (7-n): maskOrder[n] is the guid byte index that ReadBit() n fills.
// The reader then calls ReadByteSeq() in byteOrder, and only for indices whose mask bit came out
// set, so the bytes are appended in that same order.
static void WritePackedMailbox(WorldPacket& packet, ObjectGuid mailbox, uint8 const (&maskOrder)[8], uint8 const (&byteOrder)[8])
{
    uint64 raw = mailbox.GetRawValue();

    uint8 bytes[8];
    for (uint8 i = 0; i < 8; ++i)
        bytes[i] = uint8((raw >> (i * 8)) & 0xFF);

    uint8 mask = 0;
    for (uint8 bit = 0; bit < 8; ++bit)
        if (bytes[maskOrder[bit]])
            mask |= uint8(1 << (7 - bit));

    packet << mask;

    for (uint8 i = 0; i < 8; ++i)
        if (bytes[byteOrder[i]])
            packet << bytes[byteOrder[i]];
}

// Mirrors WorldSession::HandleMailTakeMoney (core MailHandler.cpp:698-714).
static void WritePackedMailboxForMoney(WorldPacket& packet, ObjectGuid mailbox)
{
    static uint8 const maskOrder[8] = {7, 6, 3, 2, 4, 5, 0, 1};
    static uint8 const byteOrder[8] = {7, 1, 4, 0, 3, 2, 6, 5};
    WritePackedMailbox(packet, mailbox, maskOrder, byteOrder);
}

// Mirrors WorldSession::HandleMailTakeItem (core MailHandler.cpp:580-595). Note this is a different
// mask/byte order from the money variant above - do not share one table between them.
static void WritePackedMailboxForItem(WorldPacket& packet, ObjectGuid mailbox)
{
    static uint8 const maskOrder[8] = {6, 5, 2, 3, 0, 1, 4, 7};
    static uint8 const byteOrder[8] = {0, 1, 4, 2, 5, 6, 3, 7};
    WritePackedMailbox(packet, mailbox, maskOrder, byteOrder);
}

// AC MailAction.cpp:13
std::map<std::string, MailProcessor*> MailAction::processors;

// AC MailAction.cpp:15-76
class TellMailProcessor : public MailProcessor
{
public:
    bool Before(PlayerbotAI* botAI) override
    {
        botAI->TellMaster("=== Mailbox ===");
        tells.clear();
        return true;
    }

    bool Process(uint32 index, Mail* mail, PlayerbotAI* botAI) override
    {
        Player* bot = botAI->GetBot();
        time_t cur_time = time(nullptr);
        uint32 days = (cur_time - mail->deliver_time) / 3600 / 24;

        std::ostringstream out;
        out << "#" << (index + 1) << " ";
        if (!mail->money && !mail->HasItems())
            out << "|cffffffff" << mail->subject;

        if (mail->money)
        {
            out << "|cffffff00" << ChatHelper::formatMoney(mail->money);
            if (!mail->subject.empty())
                out << " |cffa0a0a0(" << mail->subject << ")";
        }

        if (mail->HasItems())
        {
            for (MailItemInfoVec::iterator i = mail->items.begin(); i != mail->items.end(); ++i)
            {
                Item* item = bot->GetMItem(i->item_guid);
                uint32 count = item ? item->GetCount() : 1;

                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->item_template))
                {
                    out << ChatHelper::FormatItem(proto, count);
                    if (!mail->subject.empty())
                        out << " |cffa0a0a0(" << mail->subject << ")";
                }
            }
        }

        out << ", |cff00ff00" << days << " day(s)";
        tells.push_front(out.str());
        return true;
    }

    bool After(PlayerbotAI* botAI) override
    {
        for (std::list<std::string>::iterator i = tells.begin(); i != tells.end(); ++i)
            botAI->TellMaster(*i);

        return true;
    }

    static TellMailProcessor instance;

private:
    std::list<std::string> tells;
};

// AC MailAction.cpp:78-155
class TakeMailProcessor : public MailProcessor
{
public:
    bool Process(uint32 /*index*/, Mail* mail, PlayerbotAI* botAI) override
    {
        Player* bot = botAI->GetBot();
        if (!CheckBagSpace(bot))
        {
            botAI->TellError("Not enough bag space");
            return false;
        }

        ObjectGuid mailbox = FindMailbox(botAI);
        if (mail->money)
        {
            std::ostringstream out;
            out << mail->subject << ", |cffffff00" << ChatHelper::formatMoney(mail->money) << "|cff00ff00 processed";
            botAI->TellMaster(out.str());

            // AC MailAction.cpp:97-100 writes (mailbox, messageID); 5.4.8 reads
            // (mailId, money, packed mailbox) - see MailHandler.cpp:689-714.
            WorldPacket packet;
            packet << uint32(mail->messageID);
            packet << uint64(mail->money);
            WritePackedMailboxForMoney(packet, mailbox);
            bot->GetSession()->HandleMailTakeMoney(packet);
            RemoveMail(bot, mail->messageID, mailbox);
        }
        else if (!mail->items.empty())
        {
            std::vector<uint32> guids;
            for (MailItemInfoVec::iterator i = mail->items.begin(); i != mail->items.end(); ++i)
                if (sObjectMgr->GetItemTemplate(i->item_template))
                    guids.push_back(i->item_guid);

            for (std::vector<uint32>::iterator i = guids.begin(); i != guids.end(); ++i)
            {
                // AC MailAction.cpp:112-123 writes (mailbox, messageID, itemGuid); 5.4.8 reads
                // (mailId, itemId, packed mailbox) - see MailHandler.cpp:571-595.
                WorldPacket packet;
                packet << uint32(mail->messageID);
                packet << uint32(*i);
                WritePackedMailboxForItem(packet, mailbox);

                Item* item = bot->GetMItem(*i);
                if (!item)
                    continue;

                std::ostringstream out;
                out << mail->subject << ", " << ChatHelper::FormatItem(item->GetTemplate()) << "|cff00ff00 processed";

                bot->GetSession()->HandleMailTakeItem(packet);
                botAI->TellMaster(out.str());
            }

            RemoveMail(bot, mail->messageID, mailbox);
        }

        return true;
    }

    static TakeMailProcessor instance;

private:
    bool CheckBagSpace(Player* bot)
    {
        uint32 totalused = 0;
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
            if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                ++totalused;

        uint32 totalfree = 16 - totalused;
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
            {
                ItemTemplate const* pBagProto = pBag->GetTemplate();
                if (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER)
                    totalfree += pBag->GetFreeSlots();
            }
        }

        return totalfree >= 2;
    }
};

// AC MailAction.cpp:157-170
class DeleteMailProcessor : public MailProcessor
{
public:
    bool Process(uint32 /*index*/, Mail* mail, PlayerbotAI* botAI) override
    {
        std::ostringstream out;
        out << "|cffffffff" << mail->subject << "|cffff0000 deleted";
        RemoveMail(botAI->GetBot(), mail->messageID, FindMailbox(botAI));
        botAI->TellMaster(out.str());
        return true;
    }

    static DeleteMailProcessor instance;
};

// AC MailAction.cpp:172-185
class ReadMailProcessor : public MailProcessor
{
public:
    bool Process(uint32 /*index*/, Mail* mail, PlayerbotAI* botAI) override
    {
        std::ostringstream out, body;
        out << "|cffffffff" << mail->subject;
        botAI->TellMaster(out.str());

        return true;
    }

    static ReadMailProcessor instance;
};

// AC MailAction.cpp:187-190
TellMailProcessor TellMailProcessor::instance;
TakeMailProcessor TakeMailProcessor::instance;
DeleteMailProcessor DeleteMailProcessor::instance;
ReadMailProcessor ReadMailProcessor::instance;

// AC MailAction.cpp:192-252
static std::map<uint32, Mail*> filterList(std::vector<Mail*> src, std::string const filter)
{
    std::map<uint32, Mail*> result;

    if (src.empty())
        return result;
    if (filter.empty() || filter == "*")
    {
        uint32 idx = 0;
        for (std::vector<Mail*>::iterator i = src.begin(); i != src.end(); ++i)
            result[idx++] = *i;

        return result;
    }

    if (filter.find("-") != std::string::npos)
    {
        std::vector<std::string> ss = split(filter, '-');
        int32 from = 0;
        int32 to = static_cast<int32>(src.size()) - 1;

        if (!ss[0].empty())
            from = atoi(ss[0].c_str()) - 1;

        if (ss.size() > 1 && !ss[1].empty())
            to = atoi(ss[1].c_str()) - 1;

        if (from < 0)
            from = 0;

        if (from > static_cast<int32>(src.size()) - 1)
            from = static_cast<int32>(src.size()) - 1;

        if (to < 0)
            to = 0;

        if (to > static_cast<int32>(src.size()) - 1)
            to = static_cast<int32>(src.size()) - 1;

        for (int32 i = from; i <= to; ++i)
            result[static_cast<uint32>(i)] = src[i];

        return result;
    }

    std::vector<std::string> ss = split(filter, ',');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        int32 idx = atoi(i->c_str()) - 1;

        if (idx < 0)
            idx = 0;

        if (idx > static_cast<int32>(src.size()) - 1)
            idx = static_cast<int32>(src.size()) - 1;

        result[static_cast<uint32>(idx)] = src[idx];
    }

    return result;
}

// AC MailAction.cpp:254-317
bool MailAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!MailProcessor::FindMailbox(botAI))
    {
        botAI->TellError("There is no mailbox nearby");
        return false;
    }

    if (processors.empty())
    {
        processors["?"] = &TellMailProcessor::instance;
        processors["take"] = &TakeMailProcessor::instance;
        processors["delete"] = &DeleteMailProcessor::instance;
        processors["read"] = &ReadMailProcessor::instance;
    }

    std::string const text = event.getParam();
    if (text.empty())
    {
        botAI->TellMaster(
            "whisper 'mail ?' to query mailbox, 'mail take/delete/read filter' to take/delete/read mails by filter");
        return false;
    }

    std::vector<std::string> ss = split(text, ' ');
    std::string const action = ss[0];
    std::string const filter = ss.size() > 1 ? ss[1] : "";

    MailProcessor* processor = processors[action];
    if (!processor)
    {
        std::ostringstream out;
        out << action << ": I don't know how to do that";
        botAI->TellMaster(out.str());
        return false;
    }

    if (!processor->Before(botAI))
        return false;

    std::vector<Mail*> mailList;
    time_t cur_time = time(nullptr);
    // AC iterates Player::GetMails(); 5.4.8 only exposes GetMailBegin()/GetMailEnd() (Player.h:1983-1995).
    for (PlayerMails::iterator itr = bot->GetMailBegin(); itr != bot->GetMailEnd(); ++itr)
    {
        if ((*itr)->state == MAIL_STATE_DELETED || cur_time < (*itr)->deliver_time)
            continue;

        Mail* mail = *itr;
        mailList.push_back(mail);
    }

    std::map<uint32, Mail*> filtered = filterList(mailList, filter);
    for (std::map<uint32, Mail*>::iterator i = filtered.begin(); i != filtered.end(); ++i)
    {
        if (!processor->Process(i->first, i->second, botAI))
            break;
    }

    return processor->After(botAI);
}

// AC MailAction.cpp:319-326
void MailProcessor::RemoveMail(Player* bot, uint32 id, ObjectGuid /*mailbox*/)
{
    // AC writes (mailbox, mailId, mailTemplateId); 5.4.8's HandleMailDelete (MailHandler.cpp:459-464)
    // reads mailId then skips mailTemplateId and never reads the guid, so the guid is dropped here.
    WorldPacket packet;
    packet << uint32(id);
    packet << uint32(0);  // mailTemplateId
    bot->GetSession()->HandleMailDelete(packet);
}

// AC MailAction.cpp:328-343
ObjectGuid MailProcessor::FindMailbox(PlayerbotAI* botAI)
{
    GuidVector gos = *botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects");
    ObjectGuid mailbox;
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                mailbox = go->GetGUID();
                break;
            }
    }

    return mailbox;
}