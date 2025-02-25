#include "abstract_engine.h"

namespace DPP
{
    void AbstractEngine::run()
    {
        for( const auto& calc : m_calcs )
        {
            switch (calc.m_calc)
            {
            case Calculation::PV :
                calcPV( calc ) ;
                break;
            case Calculation::Delta :
                calcDelta( calc );
                break;
            case Calculation::Gamma :
                calcGamma( calc );
                break;
            case Calculation::Rho :
                calcRho( calc );
                break;
            case Calculation::Vega :
                calcVega( calc );
                break;
            default:
                break;
            }
        }
    }
}