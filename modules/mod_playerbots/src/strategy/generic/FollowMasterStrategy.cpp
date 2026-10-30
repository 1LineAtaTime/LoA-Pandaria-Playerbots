#include "FollowMasterStrategy.h"

#include "Playerbots.h"

NextAction** FollowMasterStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("follow", 1.0f), nullptr);
}

void FollowMasterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);
    // Beyond AC (#985): follow the master to another map or instance. Off by default (AiPlayerbot.FollowTeleport = 0):
    // then no trigger is registered at all, so the engine is exactly AC's. The strategy is also in the dead engine,
    // where this has to run before the dead strategy releases the spirit and runs back to the corpse (relevance 100).
    if (sPlayerbotAIConfig->followTeleport)
        triggers.push_back(new TriggerNode("master on other map",
            NextAction::array(0, new NextAction("teleport to master", 110.0f), nullptr)));
}
