/*
 * AC-semantics Position::HasInArc for ported mod-playerbots call sites (issue #515).
 *
 * AC Position::HasInArc(arc, obj, targetRadius) (Position.cpp:148-178): the arc is the FULL cone
 * (+-arc/2) and the third argument is the target's radius, which WIDENS the cone and makes the check
 * pass outright when the target is closer than that radius. This core's Position::HasInArc(arc, obj,
 * border) (Object.cpp:1943) instead treats the third argument as a divisor (+-arc/border), so a
 * distance passed there (sightDistance, farDistance, 10.0f) gives a nearly zero cone. Ported call
 * sites that pass a radius use this helper. Header-only; included only by those call sites.
 */
#ifndef _PLAYERBOT_BOTARC_H
#define _PLAYERBOT_BOTARC_H

#include "Position.h"

#include <cmath>

inline bool BotHasInArcAC(Position const* self, float arc, Position const* obj, float targetRadius = 0.0f)
{
    // always have self in arc
    if (obj == self)
        return true;

    // move arc to range 0.. 2*pi
    arc = Position::NormalizeOrientation(arc);

    float angle = self->GetAngle(obj);
    angle -= self->GetOrientation();

    // move angle to range -pi ... +pi
    angle = Position::NormalizeOrientation(angle);
    if (angle > float(M_PI))
        angle -= 2.0f * float(M_PI);

    float lborder = -1.0f * (arc / 2.0f);
    float rborder = (arc / 2.0f);

    if (targetRadius > 0.0f)
    {
        float distSq = self->GetExactDist2dSq(obj);
        // at least a part of the target's model is in every direction
        if (distSq < targetRadius * targetRadius)
            return true;
        float angularRadius = 2.0f * std::atan(targetRadius / (2.0f * std::sqrt(distSq)));
        lborder -= angularRadius;
        rborder += angularRadius;
    }

    return (angle >= lborder) && (angle <= rborder);
}

#endif
