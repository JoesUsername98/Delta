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
            void calcPV( const CalcData& calc );
            void calcDelta( const CalcData& calc );
            void calcRho( const CalcData& calc );
            void calcVega( const CalcData& calc );
            void calcGamma( const CalcData& calc );

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