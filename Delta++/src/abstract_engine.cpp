#include "Delta++/abstract_engine.h"
#include <algorithm>

namespace DPP
{
    void AbstractEngine::run()
    {
        for( const auto& calc : m_calcs )
        {
            switch (calc.m_calc)
            {
            case Calculation::PV :
                m_results[ Calculation::PV ] = calcPV( calc ) ;
                break;
            case Calculation::Delta :
                m_results[ Calculation::Delta ] = calcDelta( calc );
                break;
            case Calculation::Gamma :
                m_results[Calculation::Gamma] = calcGamma( calc );
                break;
            case Calculation::Rho :
                m_results[Calculation::Rho] = calcRho( calc );
                break;
            case Calculation::Vega :
                m_results[Calculation::Vega] = calcVega( calc );
                break;
            default:
                break;
            }
        }
    }

    std::string AbstractEngine::getAggregatedErrors() const
    {
        std::string aggErr;
        for ( const auto& [ key , res ] : m_results )
        {
            if ( !res.has_value() ) 
            {
                if ( !aggErr.empty() ) 
                    aggErr += " ";
                aggErr += res.error();
            }
        }
        return aggErr;
	}

    bool AbstractEngine::hasAnyErrors() const
    { 
        return std::any_of( m_results.begin(), m_results.end(), [](auto const& kv) { return !kv.second.has_value(); }); 
    }

}