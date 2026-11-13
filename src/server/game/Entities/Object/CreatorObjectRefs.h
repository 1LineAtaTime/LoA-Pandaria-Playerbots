#ifndef CREATOR_OBJECT_REFS_H
#define CREATOR_OBJECT_REFS_H

#include <unordered_set>
#include <utility>

// Tracks references, not object lifetimes. A creator may detach while a timed
// object remains alive. Forget must run when an object clears its creator.
template<class Object>
class CreatorObjectRefs
{
public:
    void Track(Object* object) { _objects.insert(object); }
    void Forget(Object* object) { _objects.erase(object); }
    bool Empty() const { return _objects.empty(); }

    template<class Detach>
    void DetachAll(Detach&& detach)
    {
        // Callbacks may Forget objects again; never iterate a mutated registry.
        auto objects = std::move(_objects);
        _objects.clear();
        for (Object* object : objects)
            detach(object);
    }
private:
    std::unordered_set<Object*> _objects;
};

#endif
