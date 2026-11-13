#include "MovementInputPolicy.h"
#include "VerticalKnockbackPolicy.h"
#include <limits>
#include <iostream>

int main()
{
    using namespace MovementInputPolicy;
    float const nan = std::numeric_limits<float>::quiet_NaN();
    float const inf = std::numeric_limits<float>::infinity();
    if (!CanFallTo(10.f, 0.f) || !CanFallTo(-10.f, -20.f)) return 1;
    if (CanFallTo(0.f, 10.f) || CanFallTo(10.f, 10.f) || CanFallTo(10.f, 9.95f)) return 2;
    if (CanFallTo(10.f - 2.f, 8.f)) return 6; // Spline origin subtracts a hovering unit's offset.
    if (CanFallTo(nan, 0.f) || CanFallTo(0.f, nan) || CanFallTo(inf, 0.f) || CanFallTo(0.f, -inf)) return 3;
    if (!IsUsableVelocity(7.f) || !IsUsableVelocity(0.001f)) return 4;
    if (IsUsableVelocity(0.f) || IsUsableVelocity(-7.f) || IsUsableVelocity(nan) || IsUsableVelocity(inf)) return 5;
    if (SelectFollowVelocity(true, 0.f, 7.f) != 7.f) return 8;
    if (SelectFollowVelocity(true, nan, 7.f) != 7.f || SelectFollowVelocity(true, inf, 7.f) != 7.f) return 9;
    if (SelectFollowVelocity(true, -1.f, 7.f) != 7.f || SelectFollowVelocity(false, 14.f, 7.f) != 7.f) return 10;
    if (SelectFollowVelocity(true, 14.f, 7.f) != 14.f) return 11;
    if (IsUsableVelocity(SelectFollowVelocity(true, 0.f, 0.f))) return 12;
    bool const taxiCases[][4] = {
        {false,false,false,false}, {false,false,true,true},
        {false,true,false,true}, {false,true,true,true},
        {true,false,false,false}, {true,false,true,false},
        {true,true,false,false}, {true,true,true,false}
    };
    for (auto const& test : taxiCases)
        if (ShouldClearTaxiForTeleport(test[0],test[1],test[2])!=test[3]) return 7;
    // Live path 2323: logout at point 69/70, 0.951 yards from its endpoint.
    if (!HasCompletedTaxiLeg(69, 70, 0, 0, 0.951f)) return 13;
    if (HasCompletedTaxiLeg(68, 70, 0, 0, 0.f) ||
        HasCompletedTaxiLeg(70, 70, 0, 0, 0.f)) return 14;
    if (HasCompletedTaxiLeg(69, 70, 1, 0, 0.f) ||
        HasCompletedTaxiLeg(69, 70, 0, 0, 61.f)) return 15;
    if (HasCompletedTaxiLeg(0, 0, 0, 0, 0.f) ||
        HasCompletedTaxiLeg(0, 1, 0, 0, 0.f) ||
        HasCompletedTaxiLeg(69, 70, 0, 0, nan) ||
        HasCompletedTaxiLeg(69, 70, 0, 0, inf) ||
        HasCompletedTaxiLeg(69, 70, 0, 0, -1.f)) return 16;
    // Live Torn Earth55715: horizontal0, vertical12. Preserve its real
    // ballistic flight (~1.244s, apex~3.732yd), not an arbitrary walk speed.
    constexpr float gravity = 19.29110527f;
    float height = 12.f * 12.f / (2.f * gravity);
    int duration = VerticalKnockbackPolicy::Duration(height, gravity);
    if (duration != 1244 || std::fabs(height - 3.7322917f) > 0.001f) return 17;
    using VerticalKnockbackPolicy::AllowsZeroVelocity;
    if (!AllowsZeroVelocity(0.f,0.f,true,true,duration)) return 18;
    if (AllowsZeroVelocity(0.f,0.f,false,true,duration) || AllowsZeroVelocity(0.f,0.f,true,false,duration) ||
        AllowsZeroVelocity(0.f,0.5f,true,true,duration) || AllowsZeroVelocity(0.f,0.f,true,true,0) ||
        AllowsZeroVelocity(-1.f,0.f,true,true,duration) || AllowsZeroVelocity(nan,0.f,true,true,duration)) return 19;
    if (VerticalKnockbackPolicy::Duration(0.f,gravity) || VerticalKnockbackPolicy::Duration(-1.f,gravity) ||
        VerticalKnockbackPolicy::Duration(nan,gravity) || VerticalKnockbackPolicy::Duration(inf,gravity) ||
        VerticalKnockbackPolicy::Duration(height,0.f) || VerticalKnockbackPolicy::Duration(height,nan)) return 20;
    auto fallingLanding = VerticalKnockbackPolicy::MakePlan(12.f, 10.f, 5.f, gravity);
    float seconds = fallingLanding.duration / 1000.f;
    if (fallingLanding.duration <= duration || std::fabs(10.f + 12.f*seconds - gravity*seconds*seconds/2.f - 5.f) > 0.02f) return 21;
    auto flatLanding = VerticalKnockbackPolicy::MakePlan(12.f, 10.f, 10.f, gravity);
    if (flatLanding.duration != duration) return 22;
    if (VerticalKnockbackPolicy::MakePlan(12.f,0.f,10.f,gravity).duration ||
        VerticalKnockbackPolicy::MakePlan(0.f,0.f,0.f,gravity).duration ||
        VerticalKnockbackPolicy::MakePlan(nan,0.f,0.f,gravity).duration ||
        VerticalKnockbackPolicy::MakePlan(12.f,0.f,0.f,0.f).duration) return 23;
    std::cout << "PASS: downward falls, grounded/upward rejection, finite movement speeds\n";
}
