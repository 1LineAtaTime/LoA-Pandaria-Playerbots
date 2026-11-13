#include "CreatureDetectionPolicy.h"
#include <cmath>
#include <iostream>
int main()
{
 using CreatureDetectionPolicy::AttackDistance;
 if (AttackDistance(10, 1, 1, 90, 0, 1) != 10) return 1;
 if (AttackDistance(20, 30, 30, 90, 0, 1) != 20) return 2;
 if (AttackDistance(10, 1, 90, 90, 0, 1) != 5) return 3;
 if (AttackDistance(20, 90, 1, 90, 0, 1) != 45) return 4;
 if (AttackDistance(20, 83, 80, 80, 0, 1) != 20) return 5;
 if (AttackDistance(20, 83, 90, 80, 0, 1) != 13) return 6;
 if (AttackDistance(20, 20, 20, 90, -5, 2) != 30) return 7;
 if (AttackDistance(20, 90, 1, 90, 0, 0) != 0) return 8;
 std::cout << "PASS: starter detection, level bounds, expansion cap, aura and rate policies\n";
}
