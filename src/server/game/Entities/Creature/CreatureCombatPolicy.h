// SkyFire combat-data integration policy. Keep fork scaling behavior explicit.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace CreatureCombatPolicy
{
    struct MeleeDamage { float minimum; float maximum; std::uint32_t attackPower; };
    inline MeleeDamage Select(bool useLevelCurve, float curve, std::uint32_t curveAP,
        float authoredMin, float authoredMax, std::uint32_t authoredAP)
    {
        if (!useLevelCurve && std::isfinite(authoredMin) && std::isfinite(authoredMax) &&
            authoredMin >= 0.0f && authoredMax > 0.0f && authoredMax >= authoredMin)
            return {authoredMin, authoredMax, authoredAP};
        curve = std::isfinite(curve) ? std::max(0.0f, curve) : 0.0f;
        return {curve, curve * 1.5f, curveAP};
    }
}
