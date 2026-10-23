/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAREDVALUECONTEXT_H
#define _PLAYERBOT_SHAREDVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "QuestValues.h"

class PlayerbotAI;
class SharedValueContext : public NamedObjectContext<UntypedValue>
{
public:
    SharedValueContext() : NamedObjectContext(true)
    {
        // #257 slice H registration (AC SharedValueContext.h:62-64). The three global quest maps.
        //
        // BOTH spellings of the relation-map name are registered on purpose. AC registers only
        // "entry quest relation", but the value declares itself as "entry quest relation map"
        // (QuestValues.h:58) and AC's own QuestValues.cpp:62 asks for the other spelling, so
        // registering only one of the two silently yields an empty map depending on which name
        // a caller uses.
        creators["entry quest relation"] = &SharedValueContext::entry_quest_relation;
        creators["entry quest relation map"] = &SharedValueContext::entry_quest_relation;
        creators["quest guidp map"] = &SharedValueContext::quest_guidp_map;
        creators["quest givers"] = &SharedValueContext::quest_givers;
    }

private:
    static UntypedValue* entry_quest_relation(PlayerbotAI* botAI)
    {
        return new EntryQuestRelationMapValue(botAI);
    }
    static UntypedValue* quest_guidp_map(PlayerbotAI* botAI) { return new QuestGuidpMapValue(botAI); }
    static UntypedValue* quest_givers(PlayerbotAI* botAI) { return new QuestGiversValue(botAI); }

    // Global acess functions
public:
    static SharedValueContext* instance()
    {
        static SharedValueContext instance;
        return &instance;
    }

    template <class T>
    Value<T>* getGlobalValue(std::string const name)
    {
        NamedObjectContextList<UntypedValue> valueContexts;
        valueContexts.Add(this);
        PlayerbotAI* botAI = new PlayerbotAI();
        UntypedValue* value = valueContexts.GetContextObject(name, botAI);
        delete botAI;
        return dynamic_cast<Value<T>*>(value);
    }

    template <class T>
    Value<T>* getGlobalValue(std::string const name, std::string const param)
    {
        return getGlobalValue<T>((std::string(name) + "::" + param));
    }

    template <class T>
    Value<T>* getGlobalValue(std::string const name, uint32 param)
    {
        std::ostringstream out;
        out << param;
        return getGlobalValue<T>(name, out.str());
    }
};

#define sSharedValueContext SharedValueContext::instance()

#endif
