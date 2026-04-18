#pragma once
#include <vector>
#include <memory>

#include "abstract_engine.h"

namespace DPP
{
    class BlackScholesEngine : public AbstractEngine
    {
    public:
        virtual ~BlackScholesEngine() = default;

        static EngineCreationResult create(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        {
            if (trd.m_optionExerciseType != OptionExerciseType::European)
                return std::unexpected("BlackScholes can only handle European Exercise");
            if (!mkt.hasLocalVolSurface())
                return std::unexpected(
                    "Black-Scholes requires m_localVolSurface; use MarketData::withFlatConstantVol for flat σ.");
            if (!mkt.isEssentiallyConstantVolSurface())
                return std::unexpected(
                    "Black-Scholes analytic engine requires constant σ(T,K) (flat AH stub); general local vol smiles "
                    "are not supported.");

            return std::unique_ptr<BlackScholesEngine>(new BlackScholesEngine(mkt, trd, calc));
        }

    protected:
        BlackScholesEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
            : AbstractEngine(mkt, trd, calc) {}
        BlackScholesEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
            : AbstractEngine(mkt, trd, calc) {}

        CalculationResult calcPV(const CalcData& calc) const override;
        CalculationResult calcDelta(const CalcData& calc) const override;
        CalculationResult calcRho(const CalcData& calc) const override;
        CalculationResult calcRhoParallel(const CalcData& calc) const override;
        CalculationResult calcVega(const CalcData& calc) const override;
        CalculationResult calcGamma(const CalcData& calc) const override;

    private:
        double getD1() const;
        double getD2() const;

        double callPrice() const;
        double putPrice() const;
    };

}