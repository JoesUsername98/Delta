#pragma once

#include <unordered_map>
#include <expected>
#include <memory>
#include <string>
#include <vector>
#include <variant>

#include "enums.h"
#include "market.h"
#include "trade.h"
#include "calc.h"

namespace DPP
{
    struct CurveRhoPoint
    {
        double tenor;
        double rho;
    };

    using CurveRho = std::vector<CurveRhoPoint>;
    using EngineValue = std::variant<double, CurveRho>;
	using CalculationResult = std::expected<EngineValue, std::string>;
	using VectorDebugMap = std::unordered_map<DebugInfo, std::vector<double>>;
    using ScalarResultMap = std::unordered_map<Calculation, CalculationResult>;
    using EngineCreationResult = std::expected<std::unique_ptr<class AbstractEngine>, std::string>;

    inline std::expected<double, std::string> scalarOrError(const CalculationResult& r)
    {
        if (!r.has_value())
            return std::unexpected(r.error());
        if (const auto* p = std::get_if<double>(&r.value()))
            return *p;
        return std::unexpected("Expected scalar result");
    }

    class AbstractEngine
    {
    public:
        const MarketData& m_mkt;
        const TradeData& m_trd;
        std::vector<CalcData> m_calcs;
        ScalarResultMap m_results;
        mutable VectorDebugMap m_debugResults;

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
        virtual CalculationResult calcRhoParallel(const CalcData& calc) const { return std::unexpected("RhoParallel not implemented"); }
        virtual CalculationResult calcVega( const CalcData& calc ) const = 0;
        virtual CalculationResult calcGamma( const CalcData& calc ) const = 0;
    };
}