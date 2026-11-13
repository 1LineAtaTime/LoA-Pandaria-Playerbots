#include "AuthoredCreatureDamage.h"
#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
namespace
{
std::shared_mutex damageMutex;
std::unordered_map<std::uint32_t, float> authoredDamage;
}
bool SetAuthoredCreatureDamage(std::uint32_t entry, float value)
{
    std::unique_lock<std::shared_mutex> lock(damageMutex);
    authoredDamage.erase(entry);
    if (!std::isfinite(value) || value < 0.0f)
        return false;
    if (value > 0.0f)
        authoredDamage.emplace(entry, value);
    return true;
}
void ClearAuthoredCreatureDamage()
{
    std::unique_lock<std::shared_mutex> lock(damageMutex);
    authoredDamage.clear();
}
float GetAuthoredCreatureDamage(std::uint32_t entry)
{
    std::shared_lock<std::shared_mutex> lock(damageMutex);
    auto itr = authoredDamage.find(entry);
    return itr == authoredDamage.end() ? 0.0f : itr->second;
}
