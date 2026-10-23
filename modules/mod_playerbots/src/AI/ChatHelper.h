/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Bot/Cmd/ChatHelper.h/.cpp (#173). AC's ChatHelper is a 733-line
 * grab-bag: item/quest/spell link formatting, money/skill/quality/slot parsing, item-class parsing,
 * chat-type parsing, race/class/spec name tables, and the "Hvalue:type:...|" parser HandleCommand's
 * "do "/"queue " path uses. Only that last piece (parseValue) is needed by anything ported in this
 * pass (HandleCommand, WhoAction, HelpAction). The rest is real work for later chat-command batches
 * (info/item/quest commands, spec 3.1a "Info"/"Items"/"Quests" groups) and is intentionally left out
 * here rather than stubbed, so a future port notices the gap instead of silently no-op'ing against a
 * half-finished helper. AiObject::chat (AiObject.cpp) only needs the type and PlayerbotAI::GetChatHelper()
 * to exist; nothing in this batch calls the other methods.
 *
 * #331 updates that note: the money / spell / quality / class / slot helpers below have since landed
 * (AC :190, :286, :423, :499, :556, :564, :589, :622). The name tables AC holds as ChatHelper statics
 * stay in ChatHelper.cpp as file-local function-static maps, so this header still has no members.
 */

#ifndef _PLAYERBOT_CHATHELPER_H
#define _PLAYERBOT_CHATHELPER_H

#include "PlayerbotAIAware.h"
// #322: FormatQuestObjective takes uint32, so Define.h (this module's usual source of it - see
// PlayerbotAIBase.h, AiFactory.h) must be included here; without it the declaration does not compile.
#include "Define.h"
#include <set>
#include <string>

struct ItemTemplate;
class PlayerbotAI;
class Quest;
// #331: this forward declaration must use the core's own class key - SpellInfo is declared
// `class SpellInfo` (src/server/game/Spells/SpellInfo.h:286). Spelling it `struct` here would name a
// different MSVC-mangled type and give an LNK2019 for a symbol that does exist.
class SpellInfo;

// AC Bot/Cmd/ChatHelper.h:25 - and that is AC's ONLY definition of it: AC's own ItemVisitors.h
// (Mgr/Item/ItemVisitors.h there) carries no typedef and gets ItemIds from this header, because
// AC Mgr/Item/ItemVisitors.h:10 does `#include "ChatHelper.h"`. #335 retires this fork's second,
// pre-existing copy the same way - strategy/ItemVisitors.h:12 is deleted and that header now includes
// this one, so the fork's layout is AC's and the two spellings can never drift apart.
// #341: SpellIds moved up here as well (it used to live in strategy/ItemVisitors.h), so both id sets sit
// beside each other exactly as AC has them (AC Bot/Cmd/ChatHelper.h:25-26) and have one definition.
typedef std::set<uint32> ItemIds;
typedef std::set<uint32> SpellIds;

class ChatHelper : public PlayerbotAIAware
{
public:
    ChatHelper(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) {}

    // AC Bot/Cmd/ChatHelper.cpp:231-249. Pulls "value" out of a "...Hvalue:<type>:<value>|..." link
    // placeholder; PlayerbotAI::HandleCommand uses this to detect a "do <action>" chat link.
    static std::string parseValue(std::string const& type, std::string const& text);

    // #322. AC Bot/Cmd/ChatHelper.h:45 + :52 - the quest-link formatters every quest action slice of
    // #257 calls (AcceptQuestAction.cpp:16, ListQuestsActions.cpp:88, DropQuestAction.cpp:53/129/227/238,
    // QuestAction.cpp:157/253/260/289/324/339/365/424/453) and every BroadcastHelper quest method.
    // FormatQuestObjective came in with them because AC BroadcastHelper.cpp:310/352 calls it.
    static std::string const FormatQuest(Quest const* quest);
    static std::string const FormatQuestObjective(std::string const name, uint32 available, uint32 required);

    // #257 slice B. AC Bot/Cmd/ChatHelper.h (declaration) + ChatHelper.cpp:432-457. The item-link
    // formatter QuestAction's QuestUpdateAddItemAction (AC QuestAction.cpp:359) and
    // QuestItemPushResultAction (AC QuestAction.cpp:420) call. Defaulted count/total match AC's header.
    static std::string const FormatItem(ItemTemplate const* proto, uint32 count = 0, uint32 total = 0);

    // #331. The money / spell / item-quality / item-class / slot half of AC's ChatHelper, in AC's
    // declaration order (AC Bot/Cmd/ChatHelper.h:38-67). Static on this fork exactly as on AC, except
    // parseSpell, which AC also keeps non-static because it needs botAI->GetBot().
    static std::string const formatMoney(uint32 copper);
    // #335. AC Bot/Cmd/ChatHelper.h:39 + Bot/Cmd/ChatHelper.cpp:251-284. Reads a "##g##s##c" amount
    // out of the command text. It is the piece SendMailAction's money branch and TradeAction's
    // set-trade-gold path both need; AC's own other caller, parseableItem (AC ChatHelper.cpp:636,
    // `parseMoney(text) > 0`), has no counterpart in this fork because parseableItem itself is not
    // ported yet (see the header comment).
    static uint32 parseMoney(std::string const text);
    static ItemIds parseItems(std::string const text);
    uint32 parseSpell(std::string const text);
    static std::string const FormatSpell(SpellInfo const* spellInfo);
    static uint32 parseItemQuality(std::string const text);
    static std::string FormatItemQuality(uint32 quality);
    static bool parseItemClass(std::string const text, uint32* itemClass, uint32* itemSubClass);
    static uint32 parseSlot(std::string const text);
};

#endif
