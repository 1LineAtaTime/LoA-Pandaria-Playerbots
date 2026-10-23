/*
 * Ported from AC mod-playerbots Ai/Base/Actions/EmoteAction.cpp's TalkAction (issue #194, S8). AC keeps
 * TalkAction in the same file as EmoteAction/EmoteActionBase; this fork gives it its own file/class,
 * sharing only the free helper PlayBotTextEmote() (EmoteAction.h) instead of a common base class.
 */

#include "TalkAction.h"

#include "EmoteAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool TalkAction::Execute(Event /*event*/)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "talk target"));
    if (!target)
        target = GetTarget();

    if (!urand(0, 100))
    {
        target = nullptr;
        context->GetValue<ObjectGuid>("talk target")->Set(ObjectGuid::Empty);
        return true;
    }

    if (target)
    {
        if (Player* player = dynamic_cast<Player*>(target))
        {
            if (PlayerbotAI* targetAI = GET_PLAYERBOT_AI(player))
                targetAI->GetAiObjectContext()->GetValue<ObjectGuid>("talk target")->Set(bot->GetGUID());
        }

        context->GetValue<ObjectGuid>("talk target")->Set(target->GetGUID());
        PlayBotTextEmote(bot, GetRandomEmote(target), target);
        return true;
    }

    return false;
}

Unit* TalkAction::GetTarget()
{
    GuidVector nfp = AI_VALUE(GuidVector, "nearest friendly players");
    std::vector<Unit*> targets;
    for (ObjectGuid const& guid : nfp)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && sServerFacade->GetDistance2d(bot, unit) < sPlayerbotAIConfig->tooCloseDistance)
            targets.push_back(unit);
    }

    if (targets.empty())
        return nullptr;

    return targets[urand(0, targets.size() - 1)];
}

uint32 TalkAction::GetRandomEmote(Unit* unit)
{
    std::vector<uint32> types;
    if (!urand(0, 20))
    {
        // expressions
        types.push_back(TEXT_EMOTE_BOW);
        types.push_back(TEXT_EMOTE_RUDE);
        types.push_back(TEXT_EMOTE_CRY);
        types.push_back(TEXT_EMOTE_LAUGH);
        types.push_back(TEXT_EMOTE_POINT);
        types.push_back(TEXT_EMOTE_CHEER);
        types.push_back(TEXT_EMOTE_SHY);
        types.push_back(TEXT_EMOTE_JOKE);
    }
    else
    {
        // talk
        types.push_back(TEXT_EMOTE_TALK);
        types.push_back(TEXT_EMOTE_TALKEX);
        types.push_back(TEXT_EMOTE_TALKQ);

        if (unit && (unit->HasNpcFlag(UNIT_NPC_FLAG_TRAINER) || unit->HasNpcFlag(UNIT_NPC_FLAG_QUESTGIVER)))
            types.push_back(TEXT_EMOTE_SALUTE);
    }

    return types[urand(0, types.size() - 1)];
}
