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
    }
    public interface ITriMatBuilderWithRiskNuetralProb
    {
        public ITriMatBuilderWithPremium WithRiskNuetralProb();
    }
    public interface ITriMatBuilderWithPremium
    {
        public ITriMatBuilderWithDelta WithPremium(OptionExerciseType exerciseType);
    }
    public interface ITriMatBuilderWithDelta
    {
        public ITriMatBuilderReady WithDelta();
    }
    public interface ITriMatBuilderReady
    {
        public TriangularMatrix<TriMatNode<State>, State> Build();
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
        TriangularMatrix<TriMatNode<State>, State> _result;

        private double _upFactor;
        private double _downFactor;
        private double _interestRate;
        Func<State, double, double> _payoffStrategy;
        Func<State, State?, State?, double> _optionPricingStrategy;

        public TriangularMatrixBuilder(int time)
        {
            _result = new TriangularMatrix<TriMatNode<State>, State>(time);
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
                    TriMatNode<State>? parentHeads = time - 1 >= downMoves && time - 1 > 0 ? _result.matrix[time - 1][downMoves] : null;
                    TriMatNode<State>? heads = time + 1 >= downMoves && time + 1 < _result.matrix.Length ? _result.matrix[time + 1][downMoves] : null;
                    TriMatNode<State>? parentTails = time - 1 >= downMoves - 1 && time - 1 > 0 && downMoves - 1 > 0 ? _result.matrix[time - 1][downMoves -1 ] : null;
                    TriMatNode<State>? tails = time + 1 >= downMoves + 1 && time + 1 < _result.matrix.Length && downMoves + 1 < _result.matrix[time + 1].Length ?
                        _result.matrix[time + 1][downMoves + 1] : null;

                    _result.matrix[time][downMoves] =
                        new TriMatNode<State>(time, downMoves,
                        new State() { UnderlyingValue = initialPrice * Math.Pow(_upFactor, noOfHeads) * Math.Pow(_downFactor, downMoves) },
                        parentHeads, parentTails, heads, tails );
                }

            return this;
        }
        public ITriMatBuilderWithPayoff WithInterestRate(double constantInterestRate)
        {
            _interestRate = constantInterestRate;

            foreach (var state in _result)
                state.Data.InterestRate = constantInterestRate;

            return this;
        }
        public ITriMatBuilderWithRiskNuetralProb WithPayoff(OptionPayoffType optionType, double strikePrice)
        {
            _payoffStrategy = GetPayoffStrategy(optionType);

            foreach (var state in _result)
                state.Data.PayOff = _payoffStrategy(state.Data, strikePrice);

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
                state.Data.ProbabilityHeads = (1 + state.Data.InterestRate - _downFactor) / (_upFactor - _downFactor);

            return this;
        }

        public ITriMatBuilderWithDelta WithPremium(OptionExerciseType exerciseType)
        {
            _optionPricingStrategy = GetOptionPricingStrategy(exerciseType);
            for (int time = _result.matrix.Length - 1; time >= 0; time--)
                for (int downMoves = _result.matrix[time].Length - 1; downMoves >= 0; downMoves--)
                {
                    var node = _result.matrix[time][downMoves];
                    State? heads = node.Heads?.Data;
                    State? tails = node.Tails?.Data;

                    if (node?.Data?.PayOff is null) throw new NullReferenceException($"Payoff is not set");

                    if (time == _result.matrix.Length - 1 || exerciseType == OptionExerciseType.American)
                    {
                        node.Data.OptionValue = _optionPricingStrategy(node.Data, heads, tails);
                        continue;
                    }

                    node.Data.OptionValue = node.Data.DiscountRate *
                                              (heads.OptionValue * node.Data.ProbabilityHeads +
                                               tails.OptionValue * node.Data.ProbabilityTails);
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
                    var node = _result.matrix[time][downMoves];
                    State heads = node.Heads.Data;
                    State tails = node.Tails.Data;

                    if (heads.UnderlyingValue == tails.UnderlyingValue)
                        throw new DivideByZeroException("Underlying Value of heads and tails node are the same leading to a divide by 0 error");

                    node.Data.DeltaHedging = (heads.OptionValue - tails.OptionValue)
                                            / (heads.UnderlyingValue - tails.UnderlyingValue);
                }

            return this;
        }

        public TriangularMatrix<TriMatNode<State>, State> Build() => _result;
    }
}
