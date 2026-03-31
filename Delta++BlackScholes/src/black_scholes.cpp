#include <Delta++BlackScholes/black_scholes.h>

#include <Delta++Math/distributions.h>

#include <algorithm>
#include <cmath>

namespace
{
    double df(double r, double T) { return std::exp(-r * T); }

    double intrinsicCall(double S, double K) { return std::max(0.0, S - K); }
    double intrinsicPut(double S, double K) { return std::max(0.0, K - S); }
}

namespace DPP::BlackScholes
{
    double d1(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        const double sqrtT = std::sqrt(T);
        const double sigSqrtT = sigma * sqrtT;
        return (std::log(S / K) + (r - q + 0.5 * sigma * sigma) * T) / sigSqrtT;
    }

    double d2(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        return d1(S, K, T, r, q, sigma) - sigma * std::sqrt(T);
    }

    double callPrice(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        if (T <= 0.0)
            return intrinsicCall(S, K);
        if (sigma <= 0.0)
        {
            const double fwd = S * std::exp((r - q) * T);
            return df(r, T) * std::max(0.0, fwd - K);
        }

        const double d1v = d1(S, K, T, r, q, sigma);
        const double d2v = d2(S, K, T, r, q, sigma);
        return S * df(q, T) * DPPMath::cumDensity(d1v) - K * df(r, T) * DPPMath::cumDensity(d2v);
    }

    double putPrice(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        if (T <= 0.0)
            return intrinsicPut(S, K);
        if (sigma <= 0.0)
        {
            const double fwd = S * std::exp((r - q) * T);
            return df(r, T) * std::max(0.0, K - fwd);
        }

        const double d1v = d1(S, K, T, r, q, sigma);
        const double d2v = d2(S, K, T, r, q, sigma);
        return K * df(r, T) * DPPMath::cumDensity(-d2v) - S * df(q, T) * DPPMath::cumDensity(-d1v);
    }

    double callPriceDf(const double S, const double K, const double T, const double r, const double df_r, const double sigma)
    {
        if (T <= 0.0)
            return intrinsicCall(S, K);
        if (sigma <= 0.0)
        {
            const double fwd = S * std::exp(r * T);
            return df_r * std::max(0.0, fwd - K);
        }

        const double d1v = d1(S, K, T, r, 0.0, sigma);
        const double d2v = d2(S, K, T, r, 0.0, sigma);
        return DPPMath::cumDensity(d1v) * S - DPPMath::cumDensity(d2v) * K * df_r;
    }

    double putPriceDf(const double S, const double K, const double T, const double r, const double df_r, const double sigma)
    {
        if (T <= 0.0)
            return intrinsicPut(S, K);
        if (sigma <= 0.0)
        {
            const double fwd = S * std::exp(r * T);
            return df_r * std::max(0.0, K - fwd);
        }

        const double d1v = d1(S, K, T, r, 0.0, sigma);
        const double d2v = d2(S, K, T, r, 0.0, sigma);
        return DPPMath::cumDensity(-d2v) * K * df_r - DPPMath::cumDensity(-d1v) * S;
    }

    double callDelta(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        if (T <= 0.0 || sigma <= 0.0)
            return (S > K) ? 1.0 : 0.0;
        return df(q, T) * DPPMath::cumDensity(d1(S, K, T, r, q, sigma));
    }

    double putDelta(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        if (T <= 0.0 || sigma <= 0.0)
            return (S < K) ? -1.0 : 0.0;
        return df(q, T) * (DPPMath::cumDensity(d1(S, K, T, r, q, sigma)) - 1.0);
    }

    double vega(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        if (T <= 0.0 || sigma <= 0.0)
            return 0.0;
        const double d1v = d1(S, K, T, r, q, sigma);
        return S * df(q, T) * DPPMath::probDensity(d1v) * std::sqrt(T);
    }

    double gamma(const double S, const double K, const double T, const double r, const double q, const double sigma)
    {
        if (T <= 0.0 || sigma <= 0.0)
            return 0.0;
        const double d1v = d1(S, K, T, r, q, sigma);
        return (df(q, T) * DPPMath::probDensity(d1v)) / (S * sigma * std::sqrt(T));
    }
}

