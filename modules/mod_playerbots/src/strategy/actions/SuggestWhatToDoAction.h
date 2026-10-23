/*
 * Ported from AC mod-playerbots Ai/Base/Actions/SuggestWhatToDoAction.h (#322). Same three classes
 * (SuggestWhatToDoAction, SuggestTradeAction, SuggestDungeonAction) as AC. AC's unused
 * `const int32_t _dbc_locale` member is dropped.
 */

#ifndef _PLAYERBOT_SUGGESTWHATTODOACTION_H
#define _PLAYERBOT_SUGGESTWHATTODOACTION_H

#include "InventoryAction.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

class PlayerbotAI;

// AC SuggestWhatToDoAction.h:14-38
class SuggestWhatToDoAction : public InventoryAction
{
public:
    SuggestWhatToDoAction(PlayerbotAI* botAI, std::string const name = "suggest what to do");

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    using Suggestion = std::function<void()>;
    std::vector<Suggestion> suggestions;
    void specificQuest();
    void grindReputation();
    void grindMaterials();
    void something();
    void toxicLinks();
    void somethingToxic();
    void thunderfury();

    std::vector<uint32> GetIncompletedQuests();

private:
    static std::map<std::string, uint8> factions;
};

// AC SuggestWhatToDoAction.h:40-45
class SuggestTradeAction : public SuggestWhatToDoAction
{
public:
    SuggestTradeAction(PlayerbotAI* botAI);
    bool Execute(Event event) override;
};

// AC SuggestWhatToDoAction.h:47-54
class SuggestDungeonAction : public SuggestWhatToDoAction
{
public:
    SuggestDungeonAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;

private:
    static std::map<std::string, uint8> instances;
};

#endif
