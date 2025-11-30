#pragma once

#include <unordered_map>
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "enums.h"
#include "market.h"
#include "trade.h"
#include "calc.h"

namespace DPP
{
	using CalculationResult = std::expected<double, std::string>;
    using ScalarResultMap = std::unordered_map<Calculation, CalculationResult>;
    using EngineCreationResult = std::expected<std::unique_ptr<class AbstractEngine>, std::string>;

    class AbstractEngine
    {
    public:
        const MarketData& m_mkt;
        const TradeData& m_trd;
        std::vector<CalcData> m_calcs;
        ScalarResultMap m_results;

    public:
        AbstractEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
            : m_mkt(mkt), m_trd(trd), m_calcs{calc} {}
        AbstractEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
            : m_mkt(mkt), m_trd(trd), m_calcs(calc) {}
        virtual ~AbstractEngine() = default;
        virtual void run();
		std::string getAggregatedErrors() const;
        bool hasAnyErrors() const;

    protected:
        virtual CalculationResult calcPV( const CalcData& calc ) const = 0 ;
        virtual CalculationResult calcDelta( const CalcData& calc ) const = 0;
        virtual CalculationResult calcRho( const CalcData& calc ) const = 0;
        virtual CalculationResult calcVega( const CalcData& calc ) const = 0;
        virtual CalculationResult calcGamma( const CalcData& calc ) const = 0;
    };
}