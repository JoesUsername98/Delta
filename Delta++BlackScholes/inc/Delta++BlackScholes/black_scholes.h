#pragma once

#include <cmath>

namespace DPP::BlackScholes
{
    double d1(double S, double K, double T, double r, double q, double sigma);
    double d2(double S, double K, double T, double r, double q, double sigma);

    double callPrice(double S, double K, double T, double r, double q, double sigma);
    double putPrice(double S, double K, double T, double r, double q, double sigma);

    // Variants matching legacy engine math (explicit discount factor).
    double callPriceDf(double S, double K, double T, double r, double df_r, double sigma);
    double putPriceDf(double S, double K, double T, double r, double df_r, double sigma);

    double callDelta(double S, double K, double T, double r, double q, double sigma);
    double putDelta(double S, double K, double T, double r, double q, double sigma);

    double vega(double S, double K, double T, double r, double q, double sigma);
    double gamma(double S, double K, double T, double r, double q, double sigma);
}

