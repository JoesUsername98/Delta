#pragma once

#include <concepts>
#include <ranges>
#include <vector>

namespace DPPMath
{
    /// `n` evenly spaced samples from `a` to `b` inclusive. For `n <= 1`, returns `{ a }`.
    template<std::floating_point T>
    std::vector<T> linspace(T a, T b, int n)
    {
        if (n <= 1)
            return {a};

        const T denom = static_cast<T>(n - 1);
        return std::views::iota(0, n)
             | std::views::transform([a, b, denom](int i) -> T {
                   return a + (b - a) * (static_cast<T>(i) / denom);
               })
             | std::ranges::to<std::vector<T>>();
    }
}
