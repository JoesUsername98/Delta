#pragma once
#include <cmath>
#include <random>

namespace DPPMath
{
    double cumDensity( double z );
    double probDensity( double z );
    double box_muller( std::mt19937_64& rng );

    // Peter J. Acklam's "Direct" inverse cumulative normal approximation.
    // Returns the z such that Phi(z) = p, where Phi is the standard normal CDF.
    // For p <= 0 returns -infinity, for p >= 1 returns +infinity.
    double invCumDensity( double p );
}