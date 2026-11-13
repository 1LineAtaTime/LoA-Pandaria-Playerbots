#include "CharacterCreationPolicy.h"
#include "DeathknellIntroPolicy.h"
#include <iostream>

int main()
{
    for (bool configured : {false,true})
        for (bool pvp : {false,true})
            for (bool bypass : {false,true})
            {
                bool expected=true;
                if (pvp && !configured && !bypass) expected=false;
                if (CharacterCreationPolicy::AllowsBothFactions(configured,pvp,bypass)!=expected) return 1;
            }
    if (DeathknellIntroPolicy::ShouldAwaken(true,true,false,false)) return 2; // Keep the initial intro.
    if (!DeathknellIntroPolicy::ShouldAwaken(true,true,true,false)) return 3;
    if (!DeathknellIntroPolicy::ShouldAwaken(true,true,false,true)) return 4; // Recover stale post-intro aura.
    if (DeathknellIntroPolicy::ShouldAwaken(false,true,true,true)) return 5;
    if (DeathknellIntroPolicy::ShouldAwaken(true,false,true,true)) return 6;
    std::cout << "PASS: 8 faction-creation combinations and intro/stale-aura recovery policies\n";
}
