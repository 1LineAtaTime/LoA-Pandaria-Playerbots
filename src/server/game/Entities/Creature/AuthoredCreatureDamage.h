#pragma once
#include <cstdint>
bool SetAuthoredCreatureDamage(std::uint32_t entry, float value);
void ClearAuthoredCreatureDamage();
float GetAuthoredCreatureDamage(std::uint32_t entry);
namespace AuthoredCreatureDamagePolicy
{
inline float Combine(float runtime, float authored, bool hasDifficulty, float difficulty, float rankRate)
{
    return runtime * (authored > 0.0f ? authored : 1.0f)
        * (hasDifficulty ? difficulty : (authored > 0.0f ? rankRate : 1.0f));
}
}
