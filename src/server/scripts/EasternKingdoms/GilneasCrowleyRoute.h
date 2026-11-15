#ifndef GILNEAS_CROWLEY_ROUTE_H
#define GILNEAS_CROWLEY_ROUTE_H
#include <cstdint>

namespace GilneasCrowleyRoute
{
    // Local authored path35231 has33 points; jumps replace legs1->2 and18->19.
    // Accept only the current motion's callback; stale/duplicate informs cannot
    // skip a leg or finish a second player's ride.
    class Progress
    {
    public:
        std::uint32_t Point() const { return _point; }
        bool Finished() const { return _point > 33; }
        bool Jump() const { return _point == 2 || _point == 19; }
        bool Arrive(std::uint32_t point, bool jump)
        {
            if (Finished() || point != _point || jump != Jump())
                return false;
            ++_point;
            return true;
        }
    private:
        std::uint32_t _point = 1;
    };
}
#endif
