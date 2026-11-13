#ifndef VERTICAL_KNOCKBACK_POLICY_H
#define VERTICAL_KNOCKBACK_POLICY_H
#include <cmath>
#include <cstdint>
#include <limits>

namespace VerticalKnockbackPolicy
{
    // A vertical launch returns to its starting height after sqrt(8*h/g).
    // No horizontal distance/speed division is defined for this trajectory.
    inline std::int32_t Duration(float height, float gravity)
    {
        if (!std::isfinite(height) || height <= 0.f || !std::isfinite(gravity) || gravity <= 0.f)
            return 0;
        double milliseconds = std::sqrt(8.0 * double(height) / double(gravity)) * 1000.0;
        if (!std::isfinite(milliseconds) || milliseconds < 1.0 ||
            milliseconds > double(std::numeric_limits<std::int32_t>::max()) - 0.5)
            return 0;
        return std::int32_t(std::lround(milliseconds));
    }

    struct Plan
    {
        float amplitude = 0.f;
        std::int32_t duration = 0;
    };

    inline Plan MakePlan(float upwardVelocity, float startZ, float landingZ, float gravity)
    {
        if (!std::isfinite(upwardVelocity) || upwardVelocity <= 0.f || !std::isfinite(startZ) ||
            !std::isfinite(landingZ) || !std::isfinite(gravity) || gravity <= 0.f)
            return {};
        double discriminant = double(upwardVelocity) * upwardVelocity - 2.0 * gravity * (double(landingZ) - startZ);
        if (discriminant < 0.0) // An unreachable landing surface must not become a teleport.
            return {};
        double seconds = (upwardVelocity + std::sqrt(discriminant)) / gravity;
        double amplitude = gravity * seconds * seconds / 8.0;
        if (!std::isfinite(amplitude) || amplitude > std::numeric_limits<float>::max())
            return {};
        float height = float(amplitude);
        return {height, Duration(height, gravity)};
    }

    inline bool AllowsZeroVelocity(float velocity, float timePercent, bool verticalTwoPointArc,
        bool ordinaryParabolic, std::int32_t duration)
    {
        return velocity == 0.f && timePercent == 0.f && verticalTwoPointArc && ordinaryParabolic && duration > 0;
    }
}
#endif
