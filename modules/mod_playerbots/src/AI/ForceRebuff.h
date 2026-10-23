/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported 1:1 from AC mod-playerbots Bot/ForceRebuff.h (#628). The #173 header batch reserved only the
 * PlayerbotAI::forceRebuff member (pending/reply flags); this is AC's full state plus the "force rebuff"
 * strategy. Bodies are in AI/ForceRebuff.cpp (AC Bot/ForceRebuff.cpp). The state is driven from the same
 * places as AC: ReadyCheckAction/ForceRebuffAction/ReadyReplyAction (begin/end), the buff spell actions
 * and BuffTrigger (refresh target), Engine (per-tick buff cycle), Trigger::needCheck (every buff trigger
 * each tick during a pass) and PlayerbotAI::CastSpell (last GCD spell).
 */

#ifndef _PLAYERBOT_FORCEREBUFF_H
#define _PLAYERBOT_FORCEREBUFF_H

#include "Define.h"
#include "Strategy.h"
#include <string>
#include <vector>

class Player;
class PlayerbotAI;
class SpellInfo;

class ForceRebuffState
{
public:
    explicit ForceRebuffState(Player* owner = nullptr) : bot(owner) {}

    ForceRebuffState(ForceRebuffState const&) = delete;
    ForceRebuffState& operator=(ForceRebuffState const&) = delete;

    void Begin(bool reply);
    void End() { pending = false; }
    bool IsPending() const;
    bool ShouldReplyToReadyCheck() const { return replyToReadyCheck; }

    void NoteBuffWork();
    void NoteBuffProposed();
    void RollBuffPendingCycle() { buffPendingThisCycle = false; buffProposedThisCycle = false; }
    bool IsBuffPendingThisCycle() const { return buffPendingThisCycle; }
    bool IsBuffProposedThisCycle() const { return buffProposedThisCycle; }

    void NoteCast(SpellInfo const* spellInfo) { lastGcdSpell = spellInfo; }
    bool IsOnGlobalCooldown() const;

    bool BuffBelowRefreshTarget(int32 remaining, int32 maxDuration, uint32 baseBeforeDuration) const;

private:
    Player* bot;
    bool pending = false;
    bool replyToReadyCheck = false;
    bool buffPendingThisCycle = false;
    bool buffProposedThisCycle = false;
    SpellInfo const* lastGcdSpell = nullptr;
    uint32 beginMs = 0;
};

class ForceRebuffStrategy : public Strategy
{
public:
    ForceRebuffStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "force rebuff"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
