/* GNU GPL version 2 or later; see COPYING.md. */
#include "BattlePayAccounting.h"
#include <cstdlib>
#include <iostream>

int main()
{
    unsigned checks = 0;
    auto check = [&](bool condition, char const* name)
    {
        ++checks;
        if (!condition)
        {
            std::cerr << "FAILED: " << name << '\n';
            std::exit(1);
        }
    };
    using namespace BattlePayAccounting;
    std::uint64_t value = 0;
    check(Price(7, 33, value) && value == 5, "fractional discount rounds up to whole DP");
    check(Price(0, 0, value) && value == 0, "free product");
    check(Price(1, 99, value) && value == 1, "maximum permitted discount");
    check(Price(45, 55, value) && value == 21, "reference discount rounds to 21 DP");
    check(!Price(1, 100, value), "invalid discount");
    check(!Price(UINT64_MAX, 0, value), "price multiplication overflow");
    check(!Price(MaxBalance + 1, 0, value), "account range overflow");
    check(Price(MaxBalance, 0, value), "largest supported catalog price");
    check(WirePrice(21, value) && value == 210000, "whole DP to client price");
    check(!WirePrice(MaxBalance + 1, value), "reject invalid wire balance");
    check(CanDebit(15, 15), "exact funds at 1:1");
    check(CanDebit(0, 0), "free product with zero funds");
    check(!CanDebit(14, 15), "insufficient funds");
    check(!CanDebit(-1, 1), "negative DP cannot wrap to unsigned");
    check(!CanDebit(-1, 0), "negative DP fails closed for free products");
    check(!CanDebit(MaxBalance, UINT64_MAX), "oversized debit");
    check(!CanDebit(INT64_MAX, 1), "invalid oversized stored balance");
    check(CanCredit(MaxBalance - 15, 15), "credit reaches exact maximum");
    check(!CanCredit(MaxBalance - 14, 15), "credit cannot overflow");
    check(!CanCredit(-1, 15), "invalid wallet cannot receive automatic credit");
    check(!CanCredit(0, UINT64_MAX), "oversized credit");
    check(Display(0) == "0", "zero display");
    check(Display(10000) == "10000", "database balance is whole DP");
    check(Display(21) == "21", "discount display");
    check(Display(1) == "1", "one DP display");
    check(Display(MaxBalance) == "2147483647", "maximum balance display");
    check(CanCancel(3, 123, 123), "closing confirmation accepts outstanding token without a price");
    check(!CanCancel(3, 124, 123), "stale cancellation cannot cancel a new quote");
    check(!CanCancel(5, 123, 123), "cancellation cannot undo an accepted purchase");
    check(!CanCancel(3, 0, 0), "unset token cannot cancel a quote");
    check(CanConfirm(3, 123, 123, 210000, 21), "valid wire quote confirmation");
    check(!CanConfirm(5, 123, 123, 210000, 21), "repeated confirmation after accepting");
    check(!CanConfirm(6, 123, 123, 210000, 21), "completed purchase cannot repeat");
    check(!CanConfirm(3, 124, 123, 210000, 21), "stale or foreign token");
    check(!CanConfirm(3, 0, 0, 210000, 21), "unset token");
    check(!CanConfirm(3, 123, 123, 209999, 21), "client price cannot override quote");
    check(CanRedeemPhysicalToken(1, 1), "one physical nonstackable token");
    check(!CanRedeemPhysicalToken(2, 20), "stacked tokens cannot share one receipt");
    check(!CanRedeemPhysicalToken(1, 20), "stack merge cannot reuse a consumed receipt");
    std::cout << checks << " BattlePay accounting checks passed\n";
}
