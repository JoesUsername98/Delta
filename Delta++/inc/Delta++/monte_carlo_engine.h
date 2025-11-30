#pragma once

#include <vector>
#include <memory>
#include <expected>
#include <string>

#include "abstract_engine.h"

#include "monte_carlo_path_schemes.h"
#include "monte_carlo_payoff.h"
#include "monte_carlo_exercise.h"

namespace DPP
{
    class MonteCarloEngine : public AbstractEngine
    {
    public:
        virtual ~MonteCarloEngine() = default;

        static EngineCreationResult create(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        {
            if (calc.empty())
                return std::unexpected("No calculation data supplied");

            if (calc.front().m_pathSchemeType != PathSchemeType::Exact &&
                calc.front().m_pathSchemeType != PathSchemeType::Euler &&
                calc.front().m_pathSchemeType != PathSchemeType::Milstein)
            {
                return std::unexpected("Unsupported path scheme type");
            }

            if (trd.m_optionPayoffType != OptionPayoffType::Call &&
                trd.m_optionPayoffType != OptionPayoffType::Put)
            {
                return std::unexpected("Unsupported option payoff type");
            }

            if (trd.m_optionExerciseType != OptionExerciseType::European &&
                trd.m_optionExerciseType != OptionExerciseType::American)
            {
                return std::unexpected("Unsupported option exercise type");
            }

            return EngineCreationResult{ std::in_place, std::unique_ptr<MonteCarloEngine>(new MonteCarloEngine(mkt, trd, calc)) };
        }

    protected:
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const CalcData& calc );
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc );

        void calcPV( const CalcData& calc ) override;
        void calcDelta( const CalcData& calc ) override;
        void calcRho( const CalcData& calc ) override;
        void calcVega( const CalcData& calc ) override;
        void calcGamma( const CalcData& calc ) override;  

    private:
        void initStrategies();

        std::unique_ptr<IMCPayoff> m_payoff;
        std::unique_ptr<IMCExercise> m_exercise;
        std::unique_ptr<IPathScheme> m_scheme;
    };
}