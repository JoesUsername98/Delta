#pragma once

#include <vector>
#include <memory>

#include "abstract_engine.h"

#include "monte_carlo_path_schemes.h"
#include "monte_carlo_payoff.h"
#include "monte_carlo_exercise.h"

namespace DPP
{
    class MonteCarloEngine : public AbstractEngine
    {
    public:
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const CalcData& calc );
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc );
        virtual ~MonteCarloEngine() = default;

    protected:
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