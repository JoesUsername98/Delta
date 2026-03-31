#include <Delta++Market/implied_vol.h>

#include <Delta++BlackScholes/black_scholes.h>

#include <algorithm>
#include <cmath>

namespace
{
    double clamp(double x, double lo, double hi)
    {
        return std::max(lo, std::min(hi, x));
    }

    double bsCallPrice(double S, double K, double T, double r, double q, double vol)
    {
        return DPP::BlackScholes::callPrice(S, K, T, r, q, vol);
    }

    double bsVega(double S, double K, double T, double r, double q, double vol)
    {
        return DPP::BlackScholes::vega(S, K, T, r, q, vol);
    }
}

namespace DPP
{
    std::expected<double, std::string> impliedVolCall(
        const double callPrice,
        const double spot,
        const double strike,
        const double tYears,
        const double r,
        const double q)
    {
        if (!(spot > 0.0) || !(strike > 0.0))
            return std::unexpected("spot and strike must be > 0");
        if (!(tYears > 0.0))
            return std::unexpected("tYears must be > 0");
        if (!(callPrice > 0.0))
            return std::unexpected("callPrice must be > 0");

        const double df_r = std::exp(-r * tYears);
        const double df_q = std::exp(-q * tYears);
        const double lower = std::max(0.0, spot * df_q - strike * df_r);
        const double upper = spot * df_q; // no-arb upper bound
        if (callPrice < lower - 1e-12)
            return std::unexpected("callPrice violates no-arbitrage lower bound");
        if (callPrice > upper + 1e-12)
            return std::unexpected("callPrice violates no-arbitrage upper bound");

        // Bracket vol in [lo, hi] so that price(lo) <= callPrice <= price(hi).
        double lo = 1e-8;
        double hi = 5.0;
        double plo = bsCallPrice(spot, strike, tYears, r, q, lo) - callPrice;
        double phi = bsCallPrice(spot, strike, tYears, r, q, hi) - callPrice;

        if (plo > 0.0)
            return std::unexpected("callPrice is below BS price at near-zero vol (unexpected)");
        while (phi < 0.0 && hi < 20.0)
        {
            hi *= 2.0;
            phi = bsCallPrice(spot, strike, tYears, r, q, hi) - callPrice;
        }
        if (phi < 0.0)
            return std::unexpected("could not bracket implied vol");

        // Safeguarded Newton with bisection fallback.
        double vol = 0.2;
        vol = clamp(vol, lo, hi);
        for (int iter = 0; iter < 100; ++iter)
        {
            const double price = bsCallPrice(spot, strike, tYears, r, q, vol);
            const double f = price - callPrice;

            if (std::abs(f) < 1e-10)
                return vol;

            // Maintain bracket.
            if (f < 0.0)
                lo = vol;
            else
                hi = vol;

            const double vega = bsVega(spot, strike, tYears, r, q, vol);
            double next = 0.5 * (lo + hi);
            if (vega > 1e-12)
            {
                const double newton = vol - f / vega;
                if (newton > lo && newton < hi)
                    next = newton;
            }

            // If bracket is tight, stop.
            if ((hi - lo) < 1e-12)
                return 0.5 * (lo + hi);

            vol = next;
        }

        return std::unexpected("implied vol did not converge");
    }
}

