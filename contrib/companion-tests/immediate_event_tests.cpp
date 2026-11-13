#include "ImmediateEventSummons.h"
#include <vector>

int main()
{
    using namespace ImmediateEventSummons;
    std::vector<Node> boat;
    boat.push_back({0, 1, false, 100, true});
    for (unsigned i = 1; i < 6; ++i)
        boat.push_back({i, i == 5 ? 0u : i + 1, true, 100, true});
    if (!Validate(boat)) return 1;
    auto broken = boat;
    broken[5].link = 1;
    if (Validate(broken)) return 2; // linked cycle
    broken = boat;
    broken[4].link = 99;
    if (Validate(broken)) return 3; // missing linked row
    broken = boat;
    broken.push_back({9, 0, true, 100, true});
    if (Validate(broken)) return 4; // unreachable orphan
    broken = boat;
    broken[2].id = 1;
    if (Validate(broken)) return 5; // duplicate key
    broken = boat;
    broken[2].supported = false;
    if (Validate(broken)) return 6; // unsupported row rejects whole event
    broken = boat;
    broken[0].link = 7;
    broken.push_back({7, 0, false, 100, true});
    if (Validate(broken)) return 7; // link cannot re-enter root
    if (Validate({}) || Validate({{1, 0, true, 100, true}})) return 8;
    unsigned successes = 0;
    for (unsigned sample = 0; sample < 100; ++sample)
    {
        successes += PassesChance(50, sample);
        if (!PassesChance(0, sample) || !PassesChance(100, sample)) return 9;
    }
    if (successes != 50 || !PassesChance(50, 49) || PassesChance(50, 50)) return 10;
    std::vector<Node> cannon;
    for (unsigned i = 0; i < 25; ++i)
        cannon.push_back({i, 0, false, 50, true});
    if (!Validate(cannon)) return 11;
    cannon[24].chance = 101;
    if (Validate(cannon)) return 12;
    return 0;
}
