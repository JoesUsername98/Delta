#pragma once

#include <expected>
#include <optional>
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
    // Inputs must be in consistent units. Strikes must be > 0. Call/put mids must be finite.
    std::expected<PutCallParityFit, std::string> inferDividendYieldFromPutCallParity(
        double spot,
        double tYears,
        const std::vector<double>& strikes,
        const std::vector<double>& callMids,
        const std::vector<double>& putMids);
}

