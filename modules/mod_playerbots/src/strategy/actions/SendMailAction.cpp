/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/SendMailAction.cpp (#258). Divergences:
 *  - sPlayerbotAIConfig. -> sPlayerbotAIConfig-> (this fork's config is an instance() pointer).
 *  - sPlayerbotAIConfig.botSendMailEnabled is not a field of this fork's PlayerbotAIConfig, so the
 *    key is read here with sConfigMgr->GetBoolDefault("AiPlayerbot.BotSendMailEnabled", true) -
 *    same key and same default as AC's conf/playerbots.conf.dist:788. Reading it in the .cpp avoids
 *    a PlayerbotAIConfig.h change (and its rebuild).
 *  - PlayerbotTextMgr is not ported (see #322), so AC's GetBotTextOrDefault() fallback literals are
 *    used verbatim instead. No %item/%receiver substitution is performed.
 *  - chat->parseItems() landed in ChatHelper in #331 (AC Bot/Cmd/ChatHelper.cpp:286-310), so the item
 *    branch below is live and "sendmail <item link> <player>" now mails the item 1:1 with AC.
 *    chat->parseMoney() landed in #335 (AC Bot/Cmd/ChatHelper.cpp:251-284), so the money branch below
 *    is live too and "sendmail 5g <player>" now mails money 1:1 with AC.
 */

#include "SendMailAction.h"

#include "AiObjectContext.h"
#include "ChatHelper.h"
#include "Config.h"
#include "Event.h"
#include "Item.h"
#include "ItemVisitors.h"
#include "Language.h"
#include "Mail.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "WorldSession.h"

#include <sstream>
#include <string>

// AC SendMailAction.cpp:15
bool SendMailAction::Execute(Event event)
{
    uint32 account = bot->GetSession()->GetAccountId();
    bool randomBot = sPlayerbotAIConfig->IsInRandomAccountList(account);

    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    bool mailboxFound = false;
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                mailboxFound = true;
                break;
            }
    }

    std::string const text = event.getParam();

    Player* receiver = GetMaster();
    Player* tellTo = receiver;

    if (!receiver)
        receiver = event.getOwner();

    if (!receiver || receiver == bot)
        return false;

    if (!tellTo)
        tellTo = receiver;

    // AC SendMailAction.cpp:46-52. #258: PlayerbotTextMgr not ported (#322) - AC's fallback literal.
    if (!sConfigMgr->GetBoolDefault("AiPlayerbot.BotSendMailEnabled", true))
    {
        bot->Whisper("I cannot send mail", LANG_UNIVERSAL, tellTo);
        return false;
    }

    // AC SendMailAction.cpp:54-60. #258: PlayerbotTextMgr not ported (#322) - AC's fallback literal.
    if (!mailboxFound && !randomBot)
    {
        bot->Whisper("There is no mailbox nearby", LANG_UNIVERSAL, tellTo);
        return false;
    }

    // #331: AC SendMailAction.cpp:62 `ItemIds ids = chat->parseItems(text);`. ChatHelper::parseItems is
    // now ported (AC Bot/Cmd/ChatHelper.cpp:286-310), so the item branch below runs for real instead of
    // being inert: the "|Hitem:<id>:..." link in the whisper text is scanned for ids exactly as AC does.
    // #258 had deliberately left this empty rather than substituting InventoryAction::Find(), which
    // resolves only "food"/"drink"/potion qualifiers and would have mailed the WRONG item.
    ItemIds ids = ChatHelper::parseItems(text);

    // AC SendMailAction.cpp:63-69. #258: PlayerbotTextMgr not ported (#322) - AC's fallback literal.
    if (ids.size() > 1)
    {
        bot->Whisper("You can not request more than one item", LANG_UNIVERSAL, tellTo);
        return false;
    }

    // AC SendMailAction.cpp:71-115 is the money branch, entered when no item link was found. #331 left
    // it deliberately inert (an immediate `return false`) because ChatHelper::parseMoney() was not
    // ported and there was no way to read an amount out of the command text without inventing a
    // parser. #335 ports parseMoney (AC Bot/Cmd/ChatHelper.cpp:251-284), so the branch is AC's below
    // and "sendmail 5g <player>" mails money 1:1. AC's cap/validation rules are kept exactly: no
    // parsable amount -> fall through silently, a random bot never moves money, and the bot must
    // actually hold the amount or it refuses via TellError.
    if (ids.empty())
    {
        uint32 money = ChatHelper::parseMoney(text);
        if (!money)
            return false;

        if (randomBot)
        {
            // AC SendMailAction.cpp:79-83. #258: PlayerbotTextMgr not ported (#322) - AC's fallback literal.
            bot->Whisper("I cannot send money", LANG_UNIVERSAL, tellTo);
            return false;
        }

        // AC SendMailAction.cpp:85-90. #258: PlayerbotTextMgr not ported (#322) - AC's fallback literal.
        // 5.4.8 divergence, none needed: Player::GetMoney() returns uint64 here (Player.h:1913) and
        // AC's is uint32, but the comparison and the subtraction below promote to uint64 either way.
        if (bot->GetMoney() < money)
        {
            botAI->TellError("I don't have enough money");
            return false;
        }

        // AC SendMailAction.cpp:92-98
        std::ostringstream body;
        body << "Hello, " << receiver->GetName() << ",\n";
        body << "\n";
        body << "Here is the money you asked for";
        body << "\n";
        body << "Thanks,\n";
        body << bot->GetName() << "\n";

        // AC SendMailAction.cpp:100-107. MailDraft::AddMoney takes uint64 on this core (Mail.h:141).
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        MailDraft draft("Money you asked for", body.str());
        draft.AddMoney(money);
        bot->SetMoney(bot->GetMoney() - money);
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

        CharacterDatabase.CommitTransaction(trans);

        // AC SendMailAction.cpp:109-114. #258: PlayerbotTextMgr not ported (#322) - AC's
        // "Sending mail to %receiver" with receiver->GetName() substituted in, whispered to tellTo
        // exactly as the item branch below does (AC uses botAI->TellMaster for the same line).
        std::ostringstream out;
        out << "Sending mail to " << receiver->GetName();
        bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
        return true;
    }

    // AC SendMailAction.cpp:117-123
    std::ostringstream body;
    body << "Hello, " << receiver->GetName() << ",\n";
    body << "\n";
    body << "Here are the item(s) you asked for";
    body << "\n";
    body << "Thanks,\n";
    body << bot->GetName() << "\n";

    // AC SendMailAction.cpp:125
    MailDraft draft("Item(s) you asked for", body.str());
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        // AC SendMailAction.cpp:128-131
        FindItemByIdVisitor visitor(*i);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

        std::vector<Item*> items = visitor.GetResult();
        for (Item* item : items)
        {
            // AC SendMailAction.cpp:134-143. #258: PlayerbotTextMgr not ported (#322); AC's
            // "Cannot send %item" with ChatHelper::FormatItem as the %item value.
            if (item->IsSoulBound() || item->IsConjuredConsumable())
            {
                std::ostringstream out;
                out << "Cannot send " << ChatHelper::FormatItem(item->GetTemplate());
                bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                continue;
            }

            ItemTemplate const* proto = item->GetTemplate();
            if (!proto)
                continue;

            // AC SendMailAction.cpp:149-164. #258: PlayerbotTextMgr not ported (#322); AC's
            // "%item: it is not for sale" with ChatHelper::FormatItem as the %item value.
            if (randomBot)
            {
                uint32 price = item->GetCount() * proto->SellPrice;
                if (!price)
                {
                    std::ostringstream out;
                    out << ChatHelper::FormatItem(item->GetTemplate()) << ": it is not for sale";
                    bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                    return false;
                }

                draft.AddCOD(price);
            }

            // AC SendMailAction.cpp:166-175
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB(trans);
            item->SetOwnerGUID(receiver->GetGUID());
            item->SaveToDB(trans);
            draft.AddItem(item);
            draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

            CharacterDatabase.CommitTransaction(trans);

            // AC SendMailAction.cpp:177-183. #258: PlayerbotTextMgr not ported (#322); AC's
            // "Sent mail to %receiver" with receiver->GetName() substituted in by AC.
            std::ostringstream out;
            out << "Sent mail to " << receiver->GetName();
            bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
            return true;
        }
    }

    return false;
}