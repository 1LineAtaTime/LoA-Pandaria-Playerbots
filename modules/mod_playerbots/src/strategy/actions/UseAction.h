/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * #611: the bare "use" chat action AC registers as `new UseItemAction(botAI)` (Ai/Base/ChatActionContext.h:126,282; the
 * constructor default name is "use"). This fork's UseItemAction (UseItemAction.h, cut down in #149) has no gameobject
 * path and no default name and is shared with the live food/drink/potion actions, so the AC "use" entry point lives in
 * this subclass instead: AC UseItemAction::Execute (UseItemAction.cpp:14-37) and UseGameObject (:40-52) on top of the
 * fork's self-use UseItem(). It is what RpgUseAction ("rpg use" -> DoSpecificAction("use", ...)) calls.
 */

#ifndef _PLAYERBOT_USEACTION_H
#define _PLAYERBOT_USEACTION_H

#include "UseItemAction.h"

class PlayerbotAI;

class UseAction : public UseItemAction
{
public:
    UseAction(PlayerbotAI* botAI) : UseItemAction(botAI, "use") {}

    bool Execute(Event event) override;
    // AC UseItemAction::isPossible (UseItemAction.cpp:396): getName() == "use" || item count > 0
    bool isPossible() override { return true; }

private:
    bool UseGameObject(ObjectGuid guid);
};

#endif
