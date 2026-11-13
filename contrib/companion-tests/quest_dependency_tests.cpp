#include "QuestDependencyPolicy.h"
#include <iostream>
#include <limits>

int main()
{
    using namespace QuestDependencyPolicy;
    if (QuestId(-619) != 619 || QuestId(619) != 619 || QuestId(0) != 0) return 1;
    if (QuestId(std::numeric_limits<std::int32_t>::min()) != 2147483648u) return 2;
    if (ReverseLink(-619, 618) != -618 || ReverseLink(619, 618) != 618) return 3;
    if (ReverseLink(0, 618) || ReverseLink(-619, 0xffffffffu)) return 4;
    if (Satisfied(618, false, true) || !Satisfied(618, true, true)) return 5;
    if (Satisfied(-618, false, false) || !Satisfied(-618, false, true) || !Satisfied(-618, true, true)) return 6;
    if (Satisfied(0, true, true)) return 7;
    std::cout << "PASS: signed prerequisite polarity, taken/rewarded state, zero and integer bounds\n";
}
