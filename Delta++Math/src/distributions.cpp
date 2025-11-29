#include <numbers>
#include <limits>
#include <cmath>

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

    double invCumDensity( double p )
    {
        if ( std::isnan( p ) ) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        // Clamp endpoints into the open interval (0,1) and reassign to p
        if ( p <= 0.0 ) {
            p = std::nextafter(0.0, 1.0);
        } else if ( p >= 1.0 ) {
            p = std::nextafter(1.0, 0.0);
        }

        // Coefficients for the rational approximations
        const double a1 = -3.969683028665376e+01;
        const double a2 =  2.209460984245205e+02;
        const double a3 = -2.759285104469687e+02;
        const double a4 =  1.383577518672690e+02;
        const double a5 = -3.066479806614716e+01;
        const double a6 =  2.506628277459239e+00;

        const double b1 = -5.447609879822406e+01;
        const double b2 =  1.615858368580409e+02;
        const double b3 = -1.556989798598866e+02;
        const double b4 =  6.680131188771972e+01;
        const double b5 = -1.328068155288572e+01;

        const double c1 = -7.784894002430293e-03;
        const double c2 = -3.223964580411365e-01;
        const double c3 = -2.400758277161838e+00;
        const double c4 = -2.549732539343734e+00;
        const double c5 =  4.374664141464968e+00;
        const double c6 =  2.938163982698783e+00;

        const double d1 =  7.784695709041462e-03;
        const double d2 =  3.224671290700398e-01;
        const double d3 =  2.445134137142996e+00;
        const double d4 =  3.754408661907416e+00;

        const double p_low = 0.02425;
        const double p_high = 1.0 - p_low;

        double x = 0.0;

        if ( p < p_low ) {
            // Rational approximation for lower region
            const double q = std::sqrt( -2.0 * std::log( p ) );
            x = ( ( ( ( c1 * q + c2 ) * q + c3 ) * q + c4 ) * q + c5 ) * q + c6;
            x = x / ( ( ( ( d1 * q + d2 ) * q + d3 ) * q + d4 ) * q + 1.0 );
            x = -x;
        }
        else if ( p <= p_high ) {
            // Rational approximation for central region
            const double q = p - 0.5;
            const double r = q * q;
            x = ( ( ( ( ( a1 * r + a2 ) * r + a3 ) * r + a4 ) * r + a5 ) * r + a6 ) * q;
            x = x / ( ( ( ( ( b1 * r + b2 ) * r + b3 ) * r + b4 ) * r + b5 ) * r + 1.0 );
        }
        else {
            // Rational approximation for upper region
            const double q = std::sqrt( -2.0 * std::log( 1.0 - p ) );
            x = ( ( ( ( c1 * q + c2 ) * q + c3 ) * q + c4 ) * q + c5 ) * q + c6;
            x = x / ( ( ( ( d1 * q + d2 ) * q + d3 ) * q + d4 ) * q + 1.0 );
        }

        // One Newton-Raphson correction using current CDF and PDF implementations
        const double err = cumDensity( x ) - p;
        const double u = err / probDensity( x );
        x = x - u / ( 1.0 + 0.5 * x * u );

        return x;
    }
}

