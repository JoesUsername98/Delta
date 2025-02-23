#pragma once 

#include <unordered_map>
#include <memory>
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

    class AbstractEngine
    {
        public:
            MarketData m_mkt;
            TradeData m_trd;
            std::vector<CalcData> m_calcs;
            ScalarResultMap m_results;
            ScalarErrorMap m_errors;

        public:
        //Const ref?
            AbstractEngine( MarketData mkt, TradeData trd, CalcData calc ) :
            m_mkt( mkt ), m_trd ( trd ), m_calcs { calc }
            {}
            AbstractEngine( MarketData mkt, TradeData trd, std::vector<CalcData> calc ) :
            m_mkt( mkt ), m_trd ( trd ), m_calcs ( calc )
            {}
            virtual ~AbstractEngine() = default;
            virtual void run();

        protected:
            virtual void calcPV( const CalcData& calc ) = 0;
            virtual void calcDelta( const CalcData& calc ) = 0;
            virtual void calcRho( const CalcData& calc ) = 0;
            virtual void calcVega( const CalcData& calc ) = 0;
            virtual void calcGamma( const CalcData& calc ) = 0;  
        
    };

    class BinomialEngine : public AbstractEngine
    {
        public:
        //Const ref?
            BinomialEngine( MarketData mkt, TradeData trd, CalcData calc ) :
            AbstractEngine( mkt, trd, calc )
            {}
            BinomialEngine( MarketData mkt, TradeData trd, std::vector<CalcData> calc ) :
            AbstractEngine( mkt, trd, calc )
            {}
            virtual ~BinomialEngine() = default;

        protected:
            void calcPV( const CalcData& calc ) override;
            void calcDelta( const CalcData& calc ) override;
            void calcRho( const CalcData& calc ) override;
            void calcVega( const CalcData& calc ) override;
            void calcGamma( const CalcData& calc ) override;  
    };
    class BlackScholesEngine : public AbstractEngine
    {
    public:
        //Const ref?
        BlackScholesEngine(MarketData mkt, TradeData trd, CalcData calc) :
            AbstractEngine(mkt, trd, calc)
        {}
        BlackScholesEngine(MarketData mkt, TradeData trd, std::vector<CalcData> calc) :
            AbstractEngine(mkt, trd, calc)
        {}
        virtual ~BlackScholesEngine() = default;

    protected:
        void calcPV(const CalcData& calc) override;
        void calcDelta(const CalcData& calc) override;
        void calcRho(const CalcData& calc) override;
        void calcVega(const CalcData& calc) override;
        void calcGamma(const CalcData& calc) override;

    private:
        double getD1() const;
        double getD2() const;

        double callPrice() const;
        double putPrice() const;

        //TODO abstract out to a Math library
        double cumDensity(double z) const;
        double probDensity(double z) const;
    };

    class EngineFactory
    {
    public:
        static std::unique_ptr<AbstractEngine> getEngine(CalculationMethod engType, MarketData mkt, TradeData trd, std::vector<CalcData> calc)
        {
            switch (engType)
            {
            case CalculationMethod::Binomial:
                return getEngine<BinomialEngine>(mkt, trd, calc);
            case CalculationMethod::BlackScholes:
                return getEngine<BlackScholesEngine>(mkt, trd, calc);
            default:
                throw std::exception("Undefined engine type requested");
            }
        }
        static std::unique_ptr<AbstractEngine> getEngine(CalculationMethod engType, MarketData mkt, TradeData trd, CalcData calc)
        {
            return getEngine(engType, mkt, trd, std::vector<CalcData>{ calc });
        }

        template <typename EngType>
        static std::unique_ptr<AbstractEngine> getEngine(MarketData mkt, TradeData trd, CalcData calc)
        {
            return getEngine<EngType>(mkt, trd, std::vector<CalcData>{calc});
        }
        template <typename EngType>
        static std::unique_ptr<AbstractEngine> getEngine(MarketData mkt, TradeData trd, std::vector<CalcData> calc)
        {
            static_assert(std::is_convertible<EngType*, AbstractEngine*>::value, "EngType must be derived from Abstract Engine");
            return std::make_unique<EngType>(mkt, trd, calc);
        }
    };
}