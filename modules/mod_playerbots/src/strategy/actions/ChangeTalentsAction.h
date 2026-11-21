/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ChangeTalentsAction.h. AC's commented-out premade-path helpers
 * (getPremadePaths, PickPremadePath, GetBestPremadeSpec, AutoSelectTalents) depend on the WotLK TalentPath/TalentSpec
 * classes, which have no 5.4.8 counterpart, and are left out.
 */

#ifndef _PLAYERBOT_CHANGETALENTSACTION_H
#define _PLAYERBOT_CHANGETALENTSACTION_H

#include "Action.h"

#include <string>

class PlayerbotAI;

class ChangeTalentsAction : public Action
{
public:
    ChangeTalentsAction(PlayerbotAI* botAI, std::string const name = "talents") : Action(botAI, name) {}

    bool Execute(Event event) override;

private:
    std::string TalentsHelp();
    std::string SpecList();
    std::string SpecPick(std::string param);
    std::string SpecApply(std::string param);
};

// AC registers this one in ActionContext.h ("auto talents"), but nothing runs it there (the only caller,
// PlayerbotFactory.cpp, is commented out). The level-up path here is AutoMaintenanceOnLevelupAction::AutoPickTalents.
// Kept, unregistered, like AC's.
class AutoSetTalentsAction : public ChangeTalentsAction
{
public:
    AutoSetTalentsAction(PlayerbotAI* botAI) : ChangeTalentsAction(botAI, "auto talents") {}

    bool Execute(Event event) override;
};

#endif
