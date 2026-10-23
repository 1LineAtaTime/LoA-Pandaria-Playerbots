#include "StayStrategy.h"

#include "Playerbots.h"

// Ported from AC mod-playerbots Ai/Base/Strategy/StayStrategy.cpp:10-20 (#177). Without this the
// "return to stay position"/ReturnToStayPositionAction pair added in #177 is registered but never
// scheduled, so a bot pushed away from its "stay" spot (e.g. knockback, a corpse run) never walks back.
void StayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "return to stay position",
            NextAction::array(0, new NextAction("return to stay position", ACTION_MOVE), nullptr)
        )
    );
}

NextAction** StayStrategy::getDefaultActions() { return NextAction::array(0, new NextAction("stay", 1.0f), nullptr); }

void SitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sit", NextAction::array(0, new NextAction("sit", 1.5f), nullptr)));
}
