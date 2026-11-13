#include "JaominDuelPolicy.h"
#include <limits>
int main()
{
    using JaominDuelPolicy::IsDefeatingHit;
    if (IsDefeatingHit(497,497,10)) return 1;
    if (!IsDefeatingHit(497,497,448)) return 2;
    if (IsDefeatingHit(497,497,447)) return 3;
    if (!IsDefeatingHit(497,497,497) || !IsDefeatingHit(497,497,1000)) return 4;
    if (!IsDefeatingHit(100,1000,1) || !IsDefeatingHit(101,1000,1)) return 5;
    if (IsDefeatingHit(102,1000,1)) return 6;
    if (IsDefeatingHit(0,1000,1) || IsDefeatingHit(1,0,1) || IsDefeatingHit(1,1000,0)) return 7;
    auto const max = std::numeric_limits<unsigned>::max();
    if (IsDefeatingHit(max,max,1) || !IsDefeatingHit(max,max,max)) return 8;
}
