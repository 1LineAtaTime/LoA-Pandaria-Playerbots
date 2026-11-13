#ifndef GILNEAS_CAGE_RECOVERY_H
#define GILNEAS_CAGE_RECOVERY_H
#include <cmath>
namespace GilneasCageRecovery
{
// Recover only the broken pre-release pillory checkpoint, not ordinary falls,
// later flooded Gilneas, combat, vehicles, or a legitimate active teleport.
inline bool Needed(unsigned race, unsigned map, bool lastStand, bool released,
                   bool alive, bool teleporting, bool vehicle, float x, float y, float z)
{
    return race == 22 && map == 654 && lastStand && !released && alive &&
        !teleporting && !vehicle && std::isfinite(z) && z < 35.0f &&
        (x + 1818.41f)*(x + 1818.41f) + (y - 2294.27f)*(y - 2294.27f) < 225.0f;
}
}
#endif
