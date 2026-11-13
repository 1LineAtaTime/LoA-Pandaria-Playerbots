// GPL v2 or later. Weighted random-enchantment selection.
#ifndef RANDOM_ENCHANTMENT_SELECTION_H
#define RANDOM_ENCHANTMENT_SELECTION_H
#include <cmath>
#include <cstdint>

namespace RandomEnchantmentSelection
{
// A valid non-empty pool must always produce an enchantment. Database chances
// are weights: their total may differ from100 because of rounding or merging.
template<class Pool>
std::uint32_t Choose(Pool const& pool, double unitRoll)
{
    if (!std::isfinite(unitRoll) || unitRoll < 0.0 || unitRoll > 1.0)
        return 0;
    double total = 0.0;
    for (auto const& item : pool)
        if (item.ench && std::isfinite(item.chance) && item.chance > 0.0f)
            total += item.chance;
    if (!(total > 0.0) || !std::isfinite(total))
        return 0;
    double remaining = unitRoll * total;
    std::uint32_t last = 0;
    for (auto const& item : pool)
        if (item.ench && std::isfinite(item.chance) && item.chance > 0.0f)
        {
            last = item.ench;
            remaining -= item.chance;
            if (remaining < 0.0)
                return last;
        }
    return last; // Endpoint/rounding: the final positive-weight entry, never0.
}
}
#endif
