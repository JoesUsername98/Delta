#pragma once

#include <unordered_map>

#include "enums.h"
#include "market.h"
#include "trade.h"
#include "calc.h"

#include "tri_matrix_builder.h"

namespace DPP
{
    using ScalarResultMap = std::unordered_map<Calculation, double>;
    using ScalarErrorMap = std::unordered_map<Calculation, std::string>;

    class AbstractEngine
    {
    public:
        const MarketData& m_mkt;
        const TradeData& m_trd;
        std::vector<CalcData> m_calcs;
        ScalarResultMap m_results;
        ScalarErrorMap m_errors;

    public:
        AbstractEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
            : m_mkt(mkt), m_trd(trd), m_calcs{calc} {}
        AbstractEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
            : m_mkt(mkt), m_trd(trd), m_calcs(calc) {}
        virtual ~AbstractEngine() = default;
        virtual void run();

    protected:
        virtual void calcPV( const CalcData& calc ) = 0;
        virtual void calcDelta( const CalcData& calc ) = 0;
        virtual void calcRho( const CalcData& calc ) = 0;
        virtual void calcVega( const CalcData& calc ) = 0;
        virtual void calcGamma( const CalcData& calc ) = 0;  
    };
}