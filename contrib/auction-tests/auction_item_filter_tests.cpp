#include "AuctionHouseBotItemFilter.h"
#include <cstdlib>
#include <iostream>

int main()
{
    using AuctionHouseBotItemFilter::HasRestrictedMarker;
    unsigned checks = 0;
    auto check = [&](bool condition, char const* description)
    {
        ++checks;
        if (!condition)
        {
            std::cerr << "FAILED: " << description << '\n';
            std::exit(1);
        }
    };

    check(HasRestrictedMarker("[DEPRECATED] Flawless Family Battle-Stone"), "bracketed deprecated marker");
    check(HasRestrictedMarker("QA Test Finger 500 PvP Power"), "QA test item");
    check(HasRestrictedMarker("Spare Gunship Cannon [TEST]"), "bracketed test marker");
    check(HasRestrictedMarker("OLD ITEM (DNT)"), "do not use marker");
    check(HasRestrictedMarker("unused"), "lowercase marker");
    check(HasRestrictedMarker("Item - DO NOT USE"), "phrase marker");
    check(!HasRestrictedMarker("Slatesteel Leggings"), "test inside a real name is safe");
    check(!HasRestrictedMarker("Contest Winner's Tabard"), "test inside contest is safe");
    check(!HasRestrictedMarker("Pristine Testimony"), "testimony is safe");
    check(!HasRestrictedMarker("Green Tea Leaf"), "ordinary item is safe");

    std::cout << checks << " auction item filter checks passed\n";
}
