/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * #611: see UseAction.h. Ported from AC Ai/Base/Actions/UseItemAction.cpp:14-52.
 * 5.4.8 / fork adaptations: chat->parseGameobjects -> ChatLinkParse::ParseGameobjects; chat->FormatGameobject(go) ==
 * RpgFormatWorldobject(go) (AC ChatHelper.cpp:379-385 and :387-396 are the same string); the item-on-gameobject branch
 * (AC UseItemOnGameObject) is not available because the fork's UseItem() is self-only (UseItemAction.h) - an event naming
 * both an item and a gameobject is reported as an error instead.
 */

#include "UseAction.h"
#include "ChatLinkParse.h"
#include "Event.h"
#include "GameObject.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "RpgAction.h"

bool UseAction::Execute(Event event)
{
    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", name);
    GuidVector gos = ChatLinkParse::ParseGameobjects(name);

    if (gos.empty())
    {
        if (!items.empty())
        {
            return UseItem(*items.begin());  // AC: UseItemAuto(*items.begin())
        }
    }
    else
    {
        if (items.empty())
            return UseGameObject(*gos.begin());
        // AC: else return UseItemOnGameObject(*items.begin(), *gos.begin());  (not available, see the file header)
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "use_item_none_available", "No items (or game objects) available", {}));
    return false;
}

bool UseAction::UseGameObject(ObjectGuid guid)
{
    GameObject* go = botAI->GetGameObject(guid);
    if (!go || !go->isSpawned() /* || go->GetGoState() != GO_STATE_READY*/)
        return false;

    go->Use(bot);

    std::ostringstream out;
    botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "use_gameobject",
        "Using %gameobject",
        {{"%gameobject", RpgFormatWorldobject(go)}}));
    return true;
}
