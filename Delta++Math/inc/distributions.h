#pragma once
#include <cmath>
#include <random>

namespace DPPMath
{
    double cumDensity( double z );
    double probDensity( double z );
    double box_muller( std::mt19937_64& rng );
}