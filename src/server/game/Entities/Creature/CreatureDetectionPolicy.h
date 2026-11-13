#pragma once
#include <algorithm>
namespace CreatureDetectionPolicy
{
inline float AttackDistance(float base, int creatureLevel, int targetLevel, int expansionCap, float auraAdjustment, float rate)
{
    if (rate == 0.0f)
        return 0.0f;
    float distance = base + float(creatureLevel - targetLevel) + auraAdjustment;
    if (creatureLevel > expansionCap && expansionCap >= targetLevel)
        distance = base + float(expansionCap - targetLevel);
    return std::max(5.0f, std::min(distance, 45.0f)) * rate;
}
}
