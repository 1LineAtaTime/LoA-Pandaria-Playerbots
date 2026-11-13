/* This file is part of the Legends of Azeroth Pandaria Project.
 * Licensed under the GNU GPL, version 2 or later; see COPYING.md. */
#ifndef BATTLE_PAY_ACCOUNTING_H
#define BATTLE_PAY_ACCOUNTING_H

#include <cstdint>
#include <limits>
#include <string>

namespace BattlePayAccounting
{
    // account.dp and catalog prices are whole donation points. Only the wire
    // protocol uses ten-thousandths of a point.
    constexpr std::uint64_t Precision = 10000;
    constexpr std::uint64_t MaxBalance = std::numeric_limits<std::int32_t>::max();

    inline bool Price(std::uint64_t catalogPrice, std::uint32_t discount, std::uint64_t& result)
    {
        if (discount > 99 || catalogPrice > MaxBalance)
            return false;
        // A discounted price must be payable with whole account.dp units.
        // Round fractional DP up consistently in the catalog and charge.
        result = (catalogPrice * (100 - discount) + 99) / 100;
        return result <= MaxBalance;
    }

    inline bool WirePrice(std::uint64_t wholeDp, std::uint64_t& result)
    {
        if (wholeDp > MaxBalance)
            return false;
        result = wholeDp * Precision;
        return true;
    }

    inline bool CanDebit(std::int64_t balance, std::uint64_t amount)
    {
        return balance >= 0 && std::uint64_t(balance) <= MaxBalance && amount <= MaxBalance && std::uint64_t(balance) >= amount;
    }

    inline bool CanCredit(std::int64_t balance, std::uint64_t amount)
    {
        return balance >= 0 && amount <= MaxBalance && std::uint64_t(balance) <= MaxBalance - amount;
    }

    inline std::string Display(std::uint64_t balance)
    {
        return std::to_string(balance);
    }

    inline bool CanCancel(std::uint32_t status, std::uint32_t token, std::uint32_t expectedToken)
    {
        return status == 3 && expectedToken != 0 && token == expectedToken;
    }

    inline bool CanConfirm(std::uint32_t status, std::uint32_t token, std::uint32_t expectedToken,
        std::uint64_t price, std::uint64_t expectedPrice)
    {
        std::uint64_t expectedWirePrice = 0;
        return CanCancel(status, token, expectedToken) &&
            WirePrice(expectedPrice, expectedWirePrice) && price == expectedWirePrice;
    }

    inline bool CanRedeemPhysicalToken(std::uint32_t count, std::uint32_t maxStack)
    {
        // A GUID identifies exactly one redeemable unit only for a template
        // that can never merge into or split from a stack.
        return count == 1 && maxStack == 1;
    }
}
#endif
