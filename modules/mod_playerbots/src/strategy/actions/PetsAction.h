#ifndef _PLAYERBOT_PETSACTION_H
#define _PLAYERBOT_PETSACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;

// AC parity: PlayerbotAI::PetFollow (AC Bot/PlayerbotAI.cpp:6712-6736) ported as a free helper instead
// of a PlayerbotAI member, to avoid adding it to AI/PlayerbotAI.h (about 49 module files include that
// header). Declared here
// because only ActionContext.h and PetsAction.cpp need it (ChatShortcutActions.cpp and
// UseMeetingStoneAction.cpp include PetsAction.h for it).
void PlayerbotPetFollow(Player* bot);

// AC parity (Ai/Base/Actions/PetsAction.h/.cpp): "aggressive"/"defensive"/"passive"/"stance"/
// "attack"/"follow"/"stay" pet sub-commands. Ported with public MoP CharmInfo/MotionMaster/CreatureAI
// API in place of AC's core-only CharmInfo::SetPlayerReactState/SetForcedSpell/SetForcedTargetGUID/
// RemoveStayPosition and Unit::ClearInPetCombat - none of those have a MoP equivalent that does
// anything useful here; see the research doc Part A.1/A.3 for why each one is dropped or remapped.
class PetsAction : public Action
{
public:
    PetsAction(PlayerbotAI* botAI, std::string const& defaultCmd = "") : Action(botAI, "pet"), defaultCmd(defaultCmd) {}

    bool Execute(Event event) override;

private:
    std::string defaultCmd;
};

// AC parity: applies sPlayerbotAIConfig->defaultPetStance to every controlled pet/guardian. Wired
// onto the "new pet" trigger (GenericTriggers.h) so a freshly summoned pet picks up the configured
// stance instead of its raw summon-time default.
class SetPetStanceAction : public Action
{
public:
    SetPetStanceAction(PlayerbotAI* botAI) : Action(botAI, "set pet stance") {}

    bool Execute(Event event) override;
};

#endif
