/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/CheckMailAction.cpp (#258). Divergences:
 *  - PlayerbotAIConfig::instance() -> sPlayerbotAIConfig-> (this fork's config is an instance() pointer).
 *  - PreparedStatement::SetData() -> setUInt32() (core PreparedStatement.h:73).
 *  - Player::GetMails() does not exist on 5.4.8; GetMailBegin()/GetMailEnd() (core Player.h:1983-1995).
 *  - GuildTaskMgr does not exist in this fork. See the #258 comment in ProcessMail().
 */

#include "CheckMailAction.h"

#include "CharacterDatabase.h"
#include "DatabaseEnv.h"
#include "Event.h"
#include "Item.h"
#include "Mail.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "WorldSession.h"

#include <sstream>
#include <vector>

// AC CheckMailAction.cpp:13-58
bool CheckMailAction::Execute(Event /*event*/)
{
    // #258: 5.4.8's HandleQueryNextMailTime (core MailHandler.cpp:1048) ignores recvData, same as
    // AC's, so an empty WorldPacket is what the client packet would look like.
    WorldPacket p;
    bot->GetSession()->HandleQueryNextMailTime(p);

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    std::vector<uint32> ids;
    // AC iterates Player::GetMails(); 5.4.8 only exposes GetMailBegin()/GetMailEnd().
    for (PlayerMails::iterator i = bot->GetMailBegin(); i != bot->GetMailEnd(); ++i)
    {
        Mail* mail = *i;
        if (!mail || mail->state == MAIL_STATE_DELETED)
            continue;

        Player* owner = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(mail->sender));
        if (!owner)
            continue;

        uint32 account = owner->GetSession()->GetAccountId();
        if (sPlayerbotAIConfig->IsInRandomAccountList(account))
            continue;

        ProcessMail(mail, owner, trans);
        ids.push_back(mail->messageID);
        mail->state = MAIL_STATE_DELETED;
    }

    for (uint32 id : ids)
    {
        bot->SendMailResult(id, MAIL_DELETED, MAIL_OK);

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_BY_ID);
        stmt->setUInt32(0, id);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM_BY_ID);
        stmt->setUInt32(0, id);
        trans->Append(stmt);

        bot->RemoveMail(id);
    }

    CharacterDatabase.CommitTransaction(trans);

    return true;
}

// AC CheckMailAction.cpp:60-66
bool CheckMailAction::isUseful()
{
    if (botAI->GetMaster() || !bot->GetMailSize() || bot->InBattleground())
        return false;

    return true;
}

// AC CheckMailAction.cpp:68-104
void CheckMailAction::ProcessMail(Mail* mail, Player* owner, CharacterDatabaseTransaction trans)
{
    if (mail->items.empty())
    {
        return;
    }

    if (mail->subject.find("Item(s) you asked for") != std::string::npos)
        return;

    for (MailItemInfoVec::iterator i = mail->items.begin(); i != mail->items.end(); ++i)
    {
        Item* item = bot->GetMItem(i->item_guid);
        if (!item)
            continue;

        // #258: GuildTaskMgr (AC Mgr/GuildTaskMgr.h, CheckItemTask) is not ported into this fork, so
        // the "is this a guild task item" test AC does here cannot run. The !CheckItemTask branch -
        // mail the item straight back to its sender - is taken unconditionally; when GuildTaskMgr
        // lands this call must be restored before the fallback.
        {
            std::ostringstream body;
            body << "Hello, " << owner->GetName() << ",\n";
            body << "\n";
            body << "Here are the item(s) you've sent me by mistake";
            body << "\n";
            body << "Thanks,\n";
            body << bot->GetName() << "\n";

            MailDraft draft("Item(s) you've sent me", body.str());
            draft.AddItem(item);
            bot->RemoveMItem(i->item_guid);
            draft.SendMailTo(trans, MailReceiver(owner), MailSender(bot));
            return;
        }

        bot->RemoveMItem(i->item_guid);
        item->DestroyForPlayer(bot);
    }
}