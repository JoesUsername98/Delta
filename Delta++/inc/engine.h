#pragma once 

#include <unordered_map>
#include <set>

#include "enums.h"
#include "market.h"
#include "trade.h"
#include "calc.h"

#include "tri_matrix_builder.h"

using namespace std::string_literals;

namespace DPP
{
    using ScalarResultMap = std::unordered_map<Calculation, double>;
    using ScalarErrorMap = std::unordered_map<Calculation, std::string>;

    class Engine
    {
        private:
            MarketData m_mkt;
            TradeData m_trd;
            std::vector<CalcData> m_calcs;
        
        public:
            ScalarResultMap m_results;
            ScalarErrorMap m_errors;

        private:
            void runBinomial( const CalcData& calc );
            // TODO clean up aftermath of function explicit specialisation templating
            void runBinomial_PV( const CalcData& calc );
            void runBinomial_Delta( const CalcData& calc );
            void runBinomial_Rho( const CalcData& calc );
            void runBinomial_Vega( const CalcData& calc );
            void runBinomial_Gamma( const CalcData& calc );

        public:
            Engine( MarketData mkt, TradeData trd, CalcData calc ) :
            m_mkt( mkt ), m_trd ( trd ), m_calcs { calc }
            {}
            Engine( MarketData mkt, TradeData trd, std::vector<CalcData> calc ) :
            m_mkt( mkt ), m_trd ( trd ), m_calcs ( calc )
            {}

            void run();      
    };
}