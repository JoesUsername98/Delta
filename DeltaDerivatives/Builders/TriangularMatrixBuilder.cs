using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DeltaDerivatives.Builders
{

    public interface ITriMatBuilderWithUnderlyingValue
    {
        public ITriMatBuilderWithInterestRate WithUnderlyingValue(double initialPrice, double upFactor);
    }
    public interface ITriMatBuilderWithInterestRate
    {
        public ITriMatBuilderWithPayoff WithInterestRate(double constantInterestRate);
    }
    public interface ITriMatBuilderWithPayoff
    {
        public ITriMatBuilderWithRiskNuetralProb WithPayoff(OptionPayoffType optionType, double strikePrice);
        //protected Func<State, double, double> GetPayoffStrategy(OptionPayoffType type);
    }
    public interface ITriMatBuilderWithRiskNuetralProb
    {
        public ITriMatBuilderWithPremium WithRiskNuetralProb();
    }
    public interface ITriMatBuilderWithPremium
    {
        public ITriMatBuilderWithDelta WithPremium(OptionExerciseType exerciseType);
        //protected Func<State, State?, State?, double> GetOptionPricingStrategy(OptionExerciseType type);
    }
    public interface ITriMatBuilderWithDelta
    {
        public ITriMatBuilderReady WithDelta();
    }
    public interface ITriMatBuilderReady
    {
        public TriangularMatrix<State> Build();
    }

    public class TriangularMatrixBuilder :
        ITriMatBuilderWithUnderlyingValue,
        ITriMatBuilderWithInterestRate,
        ITriMatBuilderWithPayoff,
        ITriMatBuilderWithRiskNuetralProb,
        ITriMatBuilderWithPremium,
        ITriMatBuilderWithDelta,
        ITriMatBuilderReady
    {
        TriangularMatrix<State> _result;

        private double _upFactor;
        private double _downFactor;
        private double _interestRate;
        Func<State, double, double> _payoffStrategy;
        Func<State, State?, State?, double> _optionPricingStrategy;

        public TriangularMatrixBuilder(int time)
        {
            _result = new TriangularMatrix<State>(time);
        }
        public ITriMatBuilderWithInterestRate WithUnderlyingValue(double initialPrice, double upFactor)
        {
            if (initialPrice < 0) throw new ArgumentException("initialPrice cannot be 0", "initialPrice");
            if (upFactor <= 0) throw new ArgumentException("upFactor cannot be 0 or less", "upFactor");
            if (upFactor < 1 / upFactor) throw new ArgumentException("upFactor cannot be less than downFactor");

            _upFactor = upFactor;
            _downFactor = 1 / _upFactor;

            for (int time = _result.matrix.Length - 1; time >= 0; time--)
                for (int downMoves = _result.matrix[time].Length - 1; downMoves >= 0; downMoves--)
                {
                    int noOfHeads = time - downMoves;
                    _result.matrix[time][downMoves] = new State();
                    _result.matrix[time][downMoves].UnderlyingValue = initialPrice * Math.Pow(_upFactor, noOfHeads) * Math.Pow(_downFactor, downMoves);
                }

            return this;
        }
        public ITriMatBuilderWithPayoff WithInterestRate(double constantInterestRate)
        {
            _interestRate = constantInterestRate;

            foreach (var state in _result)
                state.InterestRate = constantInterestRate;

            return this;
        }
        public ITriMatBuilderWithRiskNuetralProb WithPayoff(OptionPayoffType optionType, double strikePrice)
        {
            _payoffStrategy = GetPayoffStrategy(optionType);

            foreach (var state in _result)
                state.PayOff = _payoffStrategy(state, strikePrice);

            return this;
        }
        private Func<State, double, double> GetPayoffStrategy(OptionPayoffType type)
        {
            switch (type)
            {
                case OptionPayoffType.Call:
                    return (x, K) => x.UnderlyingValue - K;
                case OptionPayoffType.Put:
                    return (x, K) => K - x.UnderlyingValue;
                default:
                    throw new ArgumentException($"No payoff strategy for type {type}", "type");
            }
        }

        public ITriMatBuilderWithPremium WithRiskNuetralProb()
        {
            if (_upFactor <= 1 + _interestRate) throw new InvalidOperationException("u > 1 + r to prevent arbitrage");
            if (1 + _interestRate <= _downFactor) throw new InvalidOperationException("1 + r > d to prevent arbitrage");

            foreach (var state in _result)
                state.ProbabilityHeads = (1 + state.InterestRate - _downFactor) / (_upFactor - _downFactor);

            return this;
        }

        public ITriMatBuilderWithDelta WithPremium(OptionExerciseType exerciseType)
        {
            _optionPricingStrategy = GetOptionPricingStrategy(exerciseType);
            for (int time = _result.matrix.Length - 1; time >= 0; time--)
                for (int downMoves = _result.matrix[time].Length - 1; downMoves >= 0; downMoves--)
                {
                    var state = _result.matrix[time][downMoves];
                    State? heads = time >= _result.matrix.Length - 1 ? null : _result.matrix[time + 1][downMoves];
                    State? tails = time >= _result.matrix.Length - 1 ? null : _result.matrix[time + 1][downMoves + 1];

                    if (state?.PayOff is null) throw new NullReferenceException($"Payoff is not set");

                    if (time == _result.matrix.Length - 1 || exerciseType == OptionExerciseType.American)
                    {
                        state.OptionValue = _optionPricingStrategy(state, heads, tails);
                        continue;
                    }

                    state.OptionValue = state.DiscountRate *
                                              (heads.OptionValue * state.ProbabilityHeads +
                                               tails.OptionValue * state.ProbabilityTails);
                }

            return this;
        }

        private Func<State, State?, State?, double> GetOptionPricingStrategy(OptionExerciseType type)
        {
            switch (type)
            {
                case OptionExerciseType.European:
                    return (curr, heads, tails) => Math.Max(curr.PayOff, 0);
                case OptionExerciseType.American:
                    return (curr, heads, tails) => Math.Max(curr.PayOff, (heads is null || tails is null) ? 0.0 :
                                              curr.DiscountRate *
                                            (heads.OptionValue * curr.ProbabilityHeads +
                                             tails.OptionValue * curr.ProbabilityTails));
                default:
                    throw new ArgumentException($"No option pricing strategy for type {type}", "type");
            }
        }

        public ITriMatBuilderReady WithDelta()
        {
            for (int time = _result.matrix.Length - 2; time >= 0; time--)
                for (int downMoves = _result.matrix[time].Length - 1; downMoves >= 0; downMoves--)
                {
                    var state = _result.matrix[time][downMoves];
                    State heads = _result.matrix[time + 1][downMoves];
                    State tails = _result.matrix[time + 1][downMoves + 1];

                    if (heads.UnderlyingValue == tails.UnderlyingValue)
                        throw new DivideByZeroException("Underlying Value of heads and tails node are the same leading to a divide by 0 error");

                    state.DeltaHedging = (heads.OptionValue - tails.OptionValue)
                                            / (heads.UnderlyingValue - tails.UnderlyingValue);
                }

            return this;
        }

        public TriangularMatrix<State> Build() => _result;
    }
}
