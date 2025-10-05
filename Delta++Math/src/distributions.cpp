#include <numbers>

#include "Delta++Math/distributions.h"

namespace DPPMath
{
    double cumDensity( double z ) 
    {
        const double p = 0.3275911;
        const double a1 = 0.254829592;
        const double a2 = -0.284496736;
        const double a3 = 1.421413741;
        const double a4 = -1.453152027;
        const double a5 = 1.061405429;
        
        const int sign = z < 0.0 ? -1 : 1;

        const double x = std::abs( z ) / std::sqrt( 2.0 );
        const double t = 1.0 / (1.0 + p * x);
        const double erf = 1.0 - ( ( ( ( ( a5 * t + a4 ) * t) + a3 ) * t + a2 ) * t + a1 ) * t * std::exp( -x * x );
        return 0.5 * ( 1.0 + sign * erf );
    }

    double probDensity( double z ) 
    {
        const double inv_sqrt_2pi = 0.3989422804014337; // 1 / sqrt(2 * pi)
        return inv_sqrt_2pi * std::exp( -0.5 * z * z );
    }

    double box_muller(std::mt19937_64& rng)
    {
        static thread_local bool has_spare = false;
        static thread_local double spare{};

        if ( has_spare ) {
            has_spare = false;
            return spare;
        }

        std::uniform_real_distribution<double> unif( 0.0, 1.0 );

        const double u1 = unif( rng );
        const double u2 = unif( rng );

        const double r = std::sqrt( -2.0 * std::log( u1 ) );
        const double theta = 2.0 * std::numbers::pi_v<double> * u2;

        spare = r * std::sin( theta );
        has_spare = true;

        return r * std::cos( theta );
    }
}

