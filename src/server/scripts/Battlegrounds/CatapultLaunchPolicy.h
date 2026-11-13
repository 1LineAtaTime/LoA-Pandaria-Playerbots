#ifndef CATAPULT_LAUNCH_POLICY_H
#define CATAPULT_LAUNCH_POLICY_H
#include <cmath>
namespace CatapultLaunchPolicy
{
    inline bool Valid(bool destination, float x, float y, float z, float xy, float vertical)
    {
        return destination && std::isfinite(x) && std::isfinite(y) && std::isfinite(z) &&
            std::isfinite(xy) && xy > 0.0f && std::isfinite(vertical);
    }
}
#endif
