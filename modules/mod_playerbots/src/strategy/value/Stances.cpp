/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Value/Stances.cpp (#177). See the header comment in Stances.h
 * for the file-static-map workaround replacing AC's persisted "stance" AI value.
 *
 * Other deviations from AC:
 *  - PlayerBotSpec::IsRanged/IsTank (this fork's static helper) instead of AC's botAI->IsRanged/IsTank
 *    (this fork's PlayerbotAI has no such members - see MovementActions.cpp's Flee() for the same
 *    substitution).
 *  - AC's SetStanceAction::Execute() pings the chosen stance's location client-side on "stance show"
 *    (botAI->Ping()); this fork has no Ping() helper, so "show" instead reports the coordinates via
 *    TellMaster(), the same pattern PositionAction.cpp already uses for "position <name> ?".
 */

#include "Stances.h"

#include <unordered_map>

#include "Event.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

Unit* Stance::GetTarget()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (target)
        return target;

    ObjectGuid pullTarget = context->GetValue<ObjectGuid>("pull target")->Get();
    if (pullTarget)
        return botAI->GetUnit(pullTarget);

    return nullptr;
}

WorldLocation Stance::GetLocation()
{
    Unit* target = GetTarget();
    if (!target)
        return Formation::NullLocation;

    return GetLocationInternal();
}

WorldLocation Stance::GetNearLocation(float angle, float distance)
{
    Unit* target = GetTarget();

    float x = target->GetPositionX() + cos(angle) * distance;
    float y = target->GetPositionY() + sin(angle) * distance;
    float z = target->GetPositionZ();

    if (bot->IsWithinLOS(x, y, z))
        return WorldLocation(bot->GetMapId(), x, y, z);

    return Formation::NullLocation;
}

WorldLocation MoveStance::GetLocationInternal()
{
    Unit* target = GetTarget();
    float distance = std::max(sPlayerbotAIConfig->meleeDistance, target->GetCombatReach());

    float angle = GetAngle();
    return GetNearLocation(angle, distance);
}

std::string const Stance::GetTargetName() { return "current target"; }

float Stance::GetMaxDistance() { return sPlayerbotAIConfig->contactDistance; }

namespace
{
    // Header-need workaround (see Stances.h): the bot's current stance choice, kept file-local
    // instead of a NamedObjectContext<Value> "stance" entry (ValueContext.h is locked for this batch
    // and nothing else reads that value on either fork).
    std::unordered_map<ObjectGuid, std::string> StanceChoiceByGuid;

    class NearStance : public MoveStance
    {
    public:
        NearStance(PlayerbotAI* botAI) : MoveStance(botAI, "near") {}

        float GetAngle() override
        {
            Unit* target = GetTarget();

            if (target->GetVictim() && target->GetVictim()->GetGUID() == bot->GetGUID())
                return target->GetOrientation();

            if (botAI->HasStrategy("behind", BOT_STATE_COMBAT))
            {
                Group* group = bot->GetGroup();
                uint32 index = 0;
                uint32 count = 0;
                if (group)
                {
                    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                    {
                        Player* member = ref->GetSource();
                        if (member == bot)
                            index = count;

                        if (member && !PlayerBotSpec::IsRanged(member) && !PlayerBotSpec::IsTank(member))
                            count++;
                    }
                }

                float angle = target->GetOrientation() + static_cast<float>(M_PI);
                if (!count)
                    return angle;

                float increment = static_cast<float>(M_PI) / 4 / count;
                return round((angle + index * increment - static_cast<float>(M_PI) / 4) * 10.0f) / 10.0f;
            }

            float angle = GetFollowAngle() + target->GetOrientation();
            if (Player* master = GetMaster())
                angle -= master->GetOrientation();

            return angle;
        }
    };

    class TankStance : public MoveStance
    {
    public:
        TankStance(PlayerbotAI* botAI) : MoveStance(botAI, "tank") {}

        float GetAngle() override { return GetTarget()->GetOrientation(); }
    };

    class TurnBackStance : public MoveStance
    {
    public:
        TurnBackStance(PlayerbotAI* botAI) : MoveStance(botAI, "turnback") {}

        float GetAngle() override
        {
            Unit* target = GetTarget();

            uint32 count = 0;
            float angle = 0.0f;
            if (Group* group = bot->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    if (Player* member = ref->GetSource())
                        if (member != bot && PlayerBotSpec::IsRanged(member))
                        {
                            angle += target->GetAngle(member);
                            ++count;
                        }
                }
            }

            if (!count)
                return target->GetOrientation();

            return std::round((angle / count) * 10.0f) / 10.0f + static_cast<float>(M_PI);
        }
    };

    class BehindStance : public MoveStance
    {
    public:
        BehindStance(PlayerbotAI* botAI) : MoveStance(botAI, "behind") {}

        float GetAngle() override
        {
            Unit* target = GetTarget();

            uint32 index = 0;
            uint32 count = 0;
            if (Group* group = bot->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    if (Player* member = ref->GetSource())
                    {
                        if (member == bot)
                            index = count;

                        if (!PlayerBotSpec::IsRanged(member) && !PlayerBotSpec::IsTank(member))
                            ++count;
                    }
                }
            }

            float angle = target->GetOrientation() + static_cast<float>(M_PI);
            if (!count)
                return angle;

            float increment = static_cast<float>(M_PI) / 4 / count;
            return std::round((angle + index * increment - static_cast<float>(M_PI) / 4) * 10.0f) / 10.0f;
        }
    };

    Stance* MakeStance(PlayerbotAI* botAI, std::string const& name)
    {
        if (name == "behind")
            return new BehindStance(botAI);
        if (name == "tank")
            return new TankStance(botAI);
        if (name == "turnback" || name == "turn")
            return new TurnBackStance(botAI);
        return new NearStance(botAI); // "near" / "default" / unknown-but-valid fallback
    }

    bool IsValidStanceName(std::string const& name)
    {
        return name == "behind" || name == "near" || name == "default" || name == "tank" ||
               name == "turnback" || name == "turn";
    }
}

bool SetStanceAction::Execute(Event event)
{
    std::string const stance = event.getParam();
    std::string& current = StanceChoiceByGuid[bot->GetGUID()];
    if (current.empty())
        current = "near";

    if (stance == "?" || stance.empty())
    {
        std::ostringstream str;
        str << "Stance: |cff00ff00" << current;
        botAI->TellMaster(str);
        return true;
    }

    if (stance == "show")
    {
        std::unique_ptr<Stance> value(MakeStance(botAI, current));
        WorldLocation loc = value->GetLocation();
        if (!Formation::IsNullLocation(loc))
        {
            std::ostringstream str;
            str << "Stance " << current << " location: " << loc.GetPositionX() << "," << loc.GetPositionY();
            botAI->TellMaster(str);
        }

        return true;
    }

    if (!IsValidStanceName(stance))
    {
        std::ostringstream str;
        str << "Invalid stance: |cffff0000" << stance;
        botAI->TellMaster(str);
        botAI->TellMaster("Please set to any of:|cffffffff near (default), tank, turnback, behind");
        return false;
    }

    current = stance;

    std::ostringstream str;
    str << "Stance set to: " << stance;
    botAI->TellMaster(str);
    return true;
}
