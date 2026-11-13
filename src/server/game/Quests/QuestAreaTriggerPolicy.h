#ifndef QUEST_AREA_TRIGGER_POLICY_H
#define QUEST_AREA_TRIGGER_POLICY_H

#include <cstdint>

namespace QuestAreaTriggerPolicy
{
// Legacy quest_objective rows use 0; newer objective data uses -1 for the same
// generic scripted/exploration event. Positive values name a physical trigger.
constexpr bool IsGeneric(std::int32_t objectId) { return objectId == 0 || objectId == -1; }
constexpr bool IsSpecific(std::int32_t objectId) { return objectId > 0; }
constexpr bool Matches(std::int32_t objectId, std::uint32_t triggerId)
{
    return IsGeneric(objectId) || (IsSpecific(objectId) && std::uint32_t(objectId) == triggerId);
}
}

#endif
