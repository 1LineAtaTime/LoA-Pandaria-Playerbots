/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/TalkToQuestGiverAction.h/.cpp)
 * into the Legends of Azeroth Pandaria fork, issue #257 slice C (the master-commanded quest layer).
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h and WorldPacketHandlerStrategy.cpp
 * are owned by the single later registration slice of #257.
 */

#ifndef _PLAYERBOT_TALKTOQUESTGIVERACTION_H
#define _PLAYERBOT_TALKTOQUESTGIVERACTION_H

#include "QuestAction.h"

#include <set>
#include <sstream>
#include <string>

class PlayerbotAI;

class TalkToQuestGiverAction : public QuestAction
{
public:
    TalkToQuestGiverAction(PlayerbotAI* botAI, std::string const name = "talk to quest giver") : QuestAction(botAI, name)
    {
    }

protected:
    bool ProcessQuest(Quest const* quest, Object* questGiver) override;
    bool TurnInQuest(Quest const* quest, Object* questGiver, std::ostringstream& out);

private:
    void RewardNoItem(Quest const* quest, Object* questGiver, std::ostringstream& out);
    void RewardSingleItem(Quest const* quest, Object* questGiver, std::ostringstream& out);
    std::set<uint32> BestRewards(Quest const* quest);
    void RewardMultipleItem(Quest const* quest, Object* questGiver, std::ostringstream& out);
    void AskToSelectReward(Quest const* quest, std::ostringstream& out, bool forEquip);
};

// AC TalkToQuestGiverAction.h:33-38 / .cpp:245-306. Despite the earlier slice-C note it is not bound to a CMSG:
// it is the action of the "questgiver quest details" trigger, which AC fires on the SMSG_QUESTGIVER_OFFER_REWARD
// the server sends to the master (AC Bot/PlayerbotAI.cpp:332, mirrored at AI/PlayerbotAI.cpp:298). 5.4.8 lays
// that packet out differently from AC (bit-packed guid behind 72 dwords) - see Execute().
class TurnInQueryQuestAction : public TalkToQuestGiverAction
{
public:
    TurnInQueryQuestAction(PlayerbotAI* botAI) : TalkToQuestGiverAction(botAI, "turn in query quest") {}
    bool Execute(Event event) override;
};

#endif