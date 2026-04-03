#pragma once

#include <Delta++MarketAPI/dtos.h>

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace DPP
{
    struct PutCallParityFit
    {
        // Fit: (C - P) = A - B*K
        double A{};
        double B{};

        // Derived quantities
        double forward{}; // F = A / B
        double q{};       // from A = S * exp(-qT)

        int nUsed{};
        double rmse{};
    };

    // Infer (A,B) via OLS fit of (C-P) = A - B*K, then compute forward and q(T).
    /// Rows sorted by strike (caller may rely on SQL ORDER BY). Strikes > 0, mids finite.
    std::expected<PutCallParityFit, std::string> inferDividendYieldFromPutCallParity(
        double spot,
        double tYears,
        std::span<const PutCallMidPoint> rows);

    /// Convenience: same as span overload after zipping the three vectors (same length).
    std::expected<PutCallParityFit, std::string> inferDividendYieldFromPutCallParity(
        double spot,
        double tYears,
        const std::vector<double>& strikes,
        const std::vector<double>& callMids,
        const std::vector<double>& putMids);
}

