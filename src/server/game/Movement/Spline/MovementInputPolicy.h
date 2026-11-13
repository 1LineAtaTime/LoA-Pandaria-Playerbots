#ifndef MOVEMENT_INPUT_POLICY_H
#define MOVEMENT_INPUT_POLICY_H

#include <cmath>

namespace MovementInputPolicy
{
inline bool CanFallTo(float startZ, float groundZ)
{
    return std::isfinite(startZ) && std::isfinite(groundZ) && startZ - groundZ >= 0.1f;
}

inline bool IsUsableVelocity(float velocity)
{
    return std::isfinite(velocity) && velocity > 0.00000023841858f;
}

// A stop/facing spline can be active while carrying no translational speed.
// Followers still need their master's configured speed to close the gap.
inline float SelectFollowVelocity(bool activeSpline, float splineVelocity, float configuredVelocity)
{
    return activeSpline && IsUsableVelocity(splineVelocity) ? splineVelocity : configuredVelocity;
}

inline bool ShouldClearTaxiForTeleport(bool keepTaxi, bool flying, bool pendingRoute)
{
    return !keepTaxi && (flying || pendingRoute);
}

// The last visited point is persisted even when logout precedes the client's
// spline-done acknowledgement. Only a character at the actual final point has
// completed that leg; an out-of-range index or another map is still invalid.
inline bool HasCompletedTaxiLeg(unsigned lastPoint, unsigned pointCount,
    unsigned playerMap, unsigned endMap, float distanceToEnd)
{
    return pointCount >= 2 && lastPoint == pointCount - 1 && playerMap == endMap &&
        std::isfinite(distanceToEnd) && distanceToEnd >= 0.f && distanceToEnd <= 60.f;
}
}

#endif
