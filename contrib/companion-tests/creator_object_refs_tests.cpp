#include "CreatorObjectRefs.h"
#include <iostream>
#include <memory>

struct TimedObject { bool hasCreator=true; bool alive=true; };

int main()
{
    CreatorObjectRefs<TimedObject> refs;
    TimedObject first,second;
    refs.Track(&first);refs.Track(&first);refs.Track(&second);
    unsigned visits=0;
    refs.DetachAll([&](TimedObject* object)
    {
        ++visits;
        refs.Forget(object); // Same reentrant callback as GameObject::SetOwnerGUID.
        object->hasCreator=false;
    });
    if (visits!=2 || !refs.Empty() || first.hasCreator || second.hasCreator || !first.alive || !second.alive) return 1;
    auto expired=std::make_unique<TimedObject>();
    refs.Track(expired.get());refs.Forget(expired.get());expired.reset();
    refs.DetachAll([&](TimedObject*) {++visits;});
    if (visits!=2 || !refs.Empty()) return 2; // No access to a despawned object.
    refs.Track(&first);
    refs.DetachAll([&](TimedObject*) {refs.Track(&second);});
    if (refs.Empty()) return 3; // A callback cannot lose a newly tracked reference.
    refs.DetachAll([](TimedObject* object) {object->hasCreator=false;});
    if (!refs.Empty() || !second.alive) return 4;
    std::cout << "PASS: creator detach preserves lifetimes, deduplicates, forgets expired objects, and tolerates callbacks\n";
}
