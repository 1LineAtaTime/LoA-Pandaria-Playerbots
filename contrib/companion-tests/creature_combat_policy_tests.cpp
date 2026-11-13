#include "CreatureCombatPolicy.h"
#include "AuthoredCreatureDamage.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <limits>
int main()
{
    using CreatureCombatPolicy::Select;
    using AuthoredCreatureDamagePolicy::Combine;
    assert(Combine(1, 0, false, 1, 3) == 1); // Unmentioned local template remains unchanged.
    assert(Combine(1, 1.5f, false, 1, 2) == 3);
    assert(Combine(1, 1.5f, true, 4, 2) == 6); // Local difficulty replaces generic rank factor.
    assert(Combine(50, 0, false, 1, 3) == 50); // Runtime event behavior retained.
    assert(Combine(2, 1.5f, true, 4, 2) == 12);
    auto world = Select(false, 1, 10, 5, 9, 125);
    assert(world.minimum == 5 && world.maximum == 9 && world.attackPower == 125);
    auto scaled = Select(true, 100, 500, 5, 9, 125);
    assert(scaled.minimum == 100 && scaled.maximum == 150 && scaled.attackPower == 500);
    auto missing = Select(false, 4, 12, 0, 0, 0);
    assert(missing.minimum == 4 && missing.maximum == 6 && missing.attackPower == 12);
    auto reversed = Select(false, 4, 12, 9, 5, 1);
    assert(reversed.minimum == 4 && reversed.maximum == 6);
    auto zeroMin = Select(false, 4, 12, 0, 7, 0);
    assert(zeroMin.minimum == 0 && zeroMin.maximum == 7 && zeroMin.attackPower == 0);
    auto negative = Select(false, 4, 12, -1, 7, 0);
    assert(negative.minimum == 4);
    auto invalid = Select(false, 4, 12, 1, std::numeric_limits<float>::quiet_NaN(), 0);
    assert(invalid.minimum == 4);
    auto invalidCurve = Select(true, std::numeric_limits<float>::infinity(), 12, 1, 7, 0);
    assert(invalidCurve.minimum == 0 && invalidCurve.maximum == 0);
}
