#pragma once

#include <vector>
#include <ranges>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

#include "abstract_engine.h"

namespace DPP
{
    // Forward declaration
    class MonteCarloEngine;

    // Path generation strategy interface
    struct IPathScheme
    {
        virtual void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const = 0;
        virtual ~IPathScheme() = default;
    };

    struct ExactScheme : IPathScheme
    {
        void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const override
        {
            S = S * std::exp((mkt.m_interestRate - 0.5 * mkt.m_vol * mkt.m_vol) * dt + mkt.m_vol * dW);
        }
    };

    struct EulerScheme : IPathScheme
    {
        void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const override
        {
            S = S * (1 + mkt.m_interestRate * dt + mkt.m_vol * dW);
        }
    };

    struct MilsteinScheme : IPathScheme
    {
        void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const override
        {
            S = S * (1 + mkt.m_interestRate * dt + mkt.m_vol * dW + 0.5 * mkt.m_vol * mkt.m_vol * (dW * dW - dt));
        }
    };

    struct IMCPayoff
    {
        virtual double operator()(double s) const = 0;
        virtual ~IMCPayoff() = default;
    };

    struct MCCallPayoff : IMCPayoff
    {
        explicit MCCallPayoff(double strike) : m_strike(strike) {}
        double operator()(double s) const override { return std::max<double>(s - m_strike, 0.0); }
    private:
        double m_strike;
    };

    struct MCPutPayoff : IMCPayoff
    {
        explicit MCPutPayoff(double strike) : m_strike(strike) {}
        double operator()(double s) const override { return std::max<double>(m_strike - s, 0.0); }
    private:
        double m_strike;
    };

    struct IMCExercise
    {
        virtual double price(const MonteCarloEngine& engine, const CalcData& calc,
                             const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const = 0;
        virtual ~IMCExercise() = default;
    };

    struct MCEuropeanExercise : IMCExercise
    {
        double price(const MonteCarloEngine& engine, const CalcData& calc,
                     const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const override;
    };

    struct MCAmericanExercise : IMCExercise
    {
        double price(const MonteCarloEngine& engine, const CalcData& calc,
                     const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const override;
    };

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
        std::vector<double> simPaths(const CalcData& calc, const double dt ) const;

        const PathSchemeType m_pathSchemeType;

        std::unique_ptr<IMCPayoff> m_payoff;
        std::unique_ptr<IMCExercise> m_exercise;
        std::unique_ptr<IPathScheme> m_scheme;
    };
}