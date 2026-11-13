#include "QuestAreaTriggerPolicy.h"
#include <iostream>

int main()
{
    using namespace QuestAreaTriggerPolicy;
    // A generic event is credited only after its quest has been selected by the
    // caller's event/quest relation. An explicit trigger must match its own ID.
    if (!IsGeneric(0) || !IsGeneric(-1) || IsGeneric(-2) || IsGeneric(88)) return 1;
    if (!Matches(0, 88) || !Matches(-1, 197)) return 2;
    if (!Matches(88, 88) || Matches(88, 197) || Matches(-2, 88)) return 3;
    if (IsSpecific(0) || IsSpecific(-1) || !IsSpecific(88)) return 4;
    std::cout << "PASS: legacy event sentinels, explicit trigger matching, invalid negative IDs\n";
}
