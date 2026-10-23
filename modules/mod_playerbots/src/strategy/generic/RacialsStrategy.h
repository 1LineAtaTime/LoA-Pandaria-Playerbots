#ifndef _PLAYERBOT_RACIALSSTRATEGY_H
#define _PLAYERBOT_RACIALSSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/RacialsStrategy.{h,cpp}). The fork had no RacialsStrategy at all
// (AiFactory.cpp added the name "racials" to every engine, but nothing registered it, so it was a
// silent no-op). Extended per research (s8-pet-control-and-class-spells.md B.8) with the Cata/MoP
// Arcane Torrent variants AC predates (warrior/hunter/monk).
class RacialsStrategy : public Strategy
{
public:
    RacialsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "racials"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
