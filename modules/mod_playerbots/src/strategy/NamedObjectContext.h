/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NAMEDOBJECTCONEXT_H
#define _PLAYERBOT_NAMEDOBJECTCONEXT_H

#include <list>
#include <set>
#include <unordered_map>
#include <typeindex>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "Common.h"

class PlayerbotAI;

class Qualified
{
public:
    Qualified(){};
    Qualified(std::string const qualifier) : qualifier(qualifier) {}
    Qualified(int32 qualifier1) { Qualify(qualifier1); }

    virtual void Qualify(int qual);

    virtual void Qualify(std::string const qual) { qualifier = qual; }

    std::string const getQualifier() { return qualifier; }

    static std::string const MultiQualify(std::vector<std::string> qualifiers, const std::string& separator, const std::string_view brackets = "{}");
    static std::vector<std::string> getMultiQualifiers(std::string const qualifier1);
    static int32 getMultiQualifier(std::string const qualifier1, uint32 pos);

protected:
    std::string qualifier;
};

template <class T>
class NamedObjectFactory
{
public:
    // #72: public like AC (NamedObjectContext.h:44-47) so NamedObjectFactoryList can merge the creators.
    typedef T* (*ActionCreator)(PlayerbotAI* botAI);
    typedef std::unordered_map<std::string, ActionCreator> CreatorMap;
    CreatorMap creators;
    // #928 step 1: when set, the creators of every context of this dynamic type (one static copy shared by all bots,
    // see NamedObjectContext::ShareCreatorsByType); `creators` is then empty.
    CreatorMap const* sharedCreators = nullptr;

    CreatorMap const& Creators() const { return sharedCreators ? *sharedCreators : creators; }

public:
    T* create(std::string name, PlayerbotAI* botAI)
    {
        size_t found = name.find("::");
        std::string qualifier;
        if (found != std::string::npos)
        {
            qualifier = name.substr(found + 2);
            name = name.substr(0, found);
        }

        typename CreatorMap::const_iterator it = Creators().find(name);
        if (it == Creators().end())
            return nullptr;

        ActionCreator creator = it->second;
        if (!creator)
            return nullptr;

        T* object = (*creator)(botAI);
        Qualified* q = dynamic_cast<Qualified*>(object);
        if (q && found != std::string::npos)
            q->Qualify(qualifier);

        return object;
    }

    std::set<std::string> supports()
    {
        std::set<std::string> keys;
        for (typename CreatorMap::const_iterator it = Creators().begin(); it != Creators().end(); it++)
            keys.insert(it->first);

        return keys;
    }
};

template <class T>
class NamedObjectContext : public NamedObjectFactory<T>
{
public:
    NamedObjectContext(bool shared = false, bool supportsSiblings = false)
        : NamedObjectFactory<T>(), shared(shared), supportsSiblings(supportsSiblings)
    {
    }

    virtual ~NamedObjectContext() { Clear(); }

    T* create(std::string const name, PlayerbotAI* botAI)
    {
        if (created.find(name) == created.end())
            return created[name] = NamedObjectFactory<T>::create(name, botAI);

        return created[name];
    }

    void Clear()
    {
        for (typename std::unordered_map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
        {
            if (i->second)
                delete i->second;
        }

        created.clear();
    }

    void Update()
    {
        for (typename std::unordered_map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
        {
            if (i->second)
                i->second->Update();
        }
    }

    void Reset()
    {
        for (typename std::unordered_map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
        {
            if (i->second)
                i->second->Reset();
        }
    }

    bool IsShared() { return shared; }
    bool IsSupportsSiblings() { return supportsSiblings; }

    // #928 step 1 (memory, AC builds shared creator maps once: AiObjectContext::BuildAllSharedContexts): every bot used
    // to build each context with its own name -> creator map (~0.6 GB at 3000 bots). Contexts of one dynamic type
    // always register the same creators (constructors take no arguments and register unconditionally), so the first
    // one becomes the shared copy and later ones drop their own. Lookups and precedence are unchanged. A context whose
    // map differs in size from the shared copy keeps its own (should not happen; it only costs memory).
    void ShareCreatorsByType()
    {
        if (this->sharedCreators)
            return;

        static std::mutex registryLock;
        static std::unordered_map<std::type_index, typename NamedObjectFactory<T>::CreatorMap*> registry;

        std::lock_guard<std::mutex> guard(registryLock);
        std::type_index const type(typeid(*this));
        auto found = registry.find(type);
        if (found == registry.end())
        {
            auto* copy = new typename NamedObjectFactory<T>::CreatorMap(std::move(this->creators));
            registry.emplace(type, copy);
            this->sharedCreators = copy;
        }
        else if (found->second->size() == this->creators.size())
            this->sharedCreators = found->second;
        else
            return;

        typename NamedObjectFactory<T>::CreatorMap().swap(this->creators);
    }

    std::set<std::string> GetCreated()
    {
        std::set<std::string> keys;
        for (typename std::unordered_map<std::string, T*>::iterator it = created.begin(); it != created.end(); it++)
            keys.insert(it->first);

        return keys;
    }

protected:
    std::unordered_map<std::string, T*> created;
    bool shared;
    bool supportsSiblings;
};

template <class T>
class NamedObjectContextList
{
public:
    virtual ~NamedObjectContextList()
    {
        for (T* object : owned)
            delete object;

        for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
        {
            NamedObjectContext<T>* context = *i;
            if (!context->IsShared())
                delete context;
        }
    }

    void Add(NamedObjectContext<T>* context)
    {
        if (!context->IsShared())
            context->ShareCreatorsByType();
        contexts.push_back(context);
        // #72: a new context can answer a name that was a miss so far. #924: only the misses are dropped - the found
        // objects are owned by this list now, and the new context is last, so their first-match result stands.
        for (auto i = created.begin(); i != created.end();)
        {
            if (!i->second)
                i = created.erase(i);
            else
                ++i;
        }
    }

    // #72 perf: AC NamedObjectContextList::GetContextObject (NamedObjectContext.h:204-212) answers from one per-bot
    // name -> object map. Here every lookup walked all contexts (base + ~44 dungeon contexts + the class context
    // last), two string hashes per context, on every MultiplyAndPush -> InitializeAction -> GetAction. The objects
    // stay owned by their contexts (never cleared before the list dies), so this map is a non-owning index of the
    // first-match result the walk below returns; misses are cached too, as in AC, and Add() drops the index.
    T* GetContextObject(std::string const name, PlayerbotAI* botAI)
    {
        typename std::unordered_map<std::string, T*>::const_iterator found = created.find(name);
        if (found != created.end())
            return found->second;

        // #924: a shared context still answers from its own cache (one object for every bot); any other context only
        // creates - it used to cache every name it was asked for, misses included, so each lookup left an entry in
        // every context it walked past (~45 per name). This list now owns those objects (AC: the list owns `created`).
        T* result = nullptr;
        for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
        {
            bool const shared = (*i)->IsShared();
            T* object = shared ? (*i)->create(name, botAI) : (*i)->NamedObjectFactory<T>::create(name, botAI);
            if (object)
            {
                result = object;
                if (!shared)
                    owned.push_back(object);
                break;
            }
        }

        created[name] = result;
        return result;
    }

    void Update()
    {
        for (T* object : owned)
            object->Update();
    }

    void Reset()
    {
        for (T* object : owned)
            object->Reset();
        for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
        {
            if ((*i)->IsShared())
                (*i)->Reset();
        }
    }

    std::set<std::string> GetSiblings(std::string const name)
    {
        for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
        {
            if (!(*i)->IsSupportsSiblings())
                continue;

            std::set<std::string> supported = (*i)->supports();
            std::set<std::string>::iterator found = supported.find(name);
            if (found == supported.end())
                continue;

            supported.erase(found);
            return supported;
        }

        return std::set<std::string>();
    }

    std::set<std::string> supports()
    {
        std::set<std::string> result;
        for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
        {
            std::set<std::string> supported = (*i)->supports();

            for (std::set<std::string>::iterator j = supported.begin(); j != supported.end(); j++)
                result.insert(*j);
        }

        return result;
    }

    std::set<std::string> GetCreated()
    {
        std::set<std::string> result;
        for (auto const& entry : created)
            result.insert(entry.first);
        for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
        {
            if (!(*i)->IsShared())
                continue;
            std::set<std::string> createdKeys = (*i)->GetCreated();
            result.insert(createdKeys.begin(), createdKeys.end());
        }

        return result;
    }

private:
    std::vector<NamedObjectContext<T>*> contexts;
    std::unordered_map<std::string, T*> created;  // #72: lookup index (AC NamedObjectContextList::created), misses too
    std::vector<T*> owned;                          // #924: objects created by non-shared contexts (deleted with the list)
};

// #72 perf: AC NamedObjectFactoryList (NamedObjectContext.h:257-309) - the factories' creators are merged into one map
// on Add (a later factory overrides an earlier one, the same winner as the old push_front + first-match walk), so a
// lookup is one hash instead of a walk over every factory. Creators stay plain function pointers here.
template <class T>
class NamedObjectFactoryList
{
public:
    typedef typename NamedObjectFactory<T>::ActionCreator ObjectCreator;
    std::vector<NamedObjectFactory<T>*> factories;
    std::unordered_map<std::string, ObjectCreator> creators;

    virtual ~NamedObjectFactoryList()
    {
        for (typename std::vector<NamedObjectFactory<T>*>::const_iterator i = factories.begin(); i != factories.end(); i++)
            delete *i;
    }

    T* create(std::string name, PlayerbotAI* botAI)
    {
        size_t found = name.find("::");
        std::string qualifier;
        if (found != std::string::npos)
        {
            qualifier = name.substr(found + 2);
            name = name.substr(0, found);
        }

        typename std::unordered_map<std::string, ObjectCreator>::const_iterator creator = creators.find(name);
        if (creator == creators.end() || !creator->second)
            return nullptr;

        T* object = (*creator->second)(botAI);
        Qualified* q = dynamic_cast<Qualified*>(object);
        if (q && found != std::string::npos)
            q->Qualify(qualifier);

        return object;
    }

    void Add(NamedObjectFactory<T>* context)
    {
        factories.push_back(context);
        for (auto const& iter : context->creators)
            if (iter.second)  // the old walk fell through a null creator to the next factory
                creators[iter.first] = iter.second;
    }

    T* GetContextObject(std::string const& name, PlayerbotAI* botAI)
    {
        if (T* object = create(name, botAI))
            return object;

        return nullptr;
    }
};

#endif
