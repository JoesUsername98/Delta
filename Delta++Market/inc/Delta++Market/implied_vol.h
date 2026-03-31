#pragma once

#include <expected>
#include <string>

namespace DPP
{
    // Black–Scholes implied volatility for a European call with continuous dividend yield q.
    // Returns sigma (annualized) or an error message.
    std::expected<double, std::string> impliedVolCall(
        double callPrice,
        double spot,
        double strike,
        double tYears,
        double r,
        double q = 0); 
}

