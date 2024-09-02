using DeltaDerivatives.Factory;
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
        public ITriMatBuilderWithInterestRate WithUnderlyingValueAndUpFactor(double initialPrice, double upFactor);
        public ITriMatBuilderWithInterestRate WithUnderlyingValueAndVolatility(double initialPrice, double vol);
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
        public ITriMatBuilderWithPsuedoOptimalStoppingTime WithDelta();
    }
    public interface ITriMatBuilderWithPsuedoOptimalStoppingTime
    {
        public ITriMatBuilderReady WithPsuedoOptimalStoppingTime();
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
        ITriMatBuilderWithPsuedoOptimalStoppingTime,
        ITriMatBuilderReady
    {
        TriangularMatrix<TriMatNode<State>, State> _result;

        private int _timeSteps;
        private double _timeStep;
        private double _upFactor;
        private double _downFactor;
        private double _interestRate;
        Func<State, double, double> _payoffStrategy;
        private OptionExerciseType _exerciseType;
        Func<State, State?, State?, double> _optionPricingStrategy;

        public TriangularMatrixBuilder(int steps, double timeStep)
        {
            _timeSteps = steps;
            _timeStep = timeStep;
            _result = new TriangularMatrix<TriMatNode<State>, State>(steps, timeStep);
        }
        public ITriMatBuilderWithInterestRate WithUnderlyingValueAndVolatility(double initialPrice, double vol)
        {
            double upFactor = Math.Exp(vol * Math.Sqrt(_timeStep) );
            return WithUnderlyingValueAndUpFactor(initialPrice, upFactor);
        }
        public ITriMatBuilderWithInterestRate WithUnderlyingValueAndUpFactor(double initialPrice, double upFactor)
        {
            if (initialPrice < 0) throw new ArgumentException("initialPrice cannot be 0", "initialPrice");
            if (upFactor <= 0) throw new ArgumentException("upFactor cannot be 0 or less", "upFactor");
            if (upFactor < 1 / upFactor) throw new ArgumentException("upFactor cannot be less than downFactor");

            _upFactor = upFactor;
            _downFactor = 1 / _upFactor;

            for (int step = _result.matrix.Length - 1; step >= 0; step--)
                for (int downMoves = _result.matrix[step].Length - 1; downMoves >= 0; downMoves--)
                {
                    int noOfHeads = step - downMoves;
                    _result.matrix[step][downMoves] =
                        new TriMatNode<State>(step, downMoves,
                        new State() { UnderlyingValue = initialPrice * Math.Pow(_upFactor, noOfHeads ) * Math.Pow(_downFactor, downMoves ) }); 
                }

            for (int step = _result.matrix.Length - 1; step >= 0; step--)
                for (int downMoves = _result.matrix[step].Length - 1; downMoves >= 0; downMoves--)
                {
                    _result.matrix[step][downMoves].ParentHeads = step - 1 >= downMoves && step - 1 >= 0 ? _result.matrix[step - 1][downMoves] : null;
                    _result.matrix[step][downMoves].Heads = step + 1 >= downMoves && step + 1 < _result.matrix.Length ? _result.matrix[step + 1][downMoves] : null;
                    _result.matrix[step][downMoves].ParentTails = step >= downMoves - 1 && step - 1 >= 0 && downMoves - 1 >= 0 ? _result.matrix[step - 1][downMoves - 1] : null;
                    _result.matrix[step][downMoves].Tails = step + 1 >= downMoves + 1 && step + 1 < _result.matrix.Length && downMoves + 1 < _result.matrix[step + 1].Length ?
                        _result.matrix[step + 1][downMoves + 1] : null;
                }

            return this;
        }
        public ITriMatBuilderWithPayoff WithInterestRate(double constantInterestRate)
        {
            _interestRate = constantInterestRate;

            foreach (var state in _result)
                state.Data.InterestRate = Math.Pow(1 + constantInterestRate, _timeStep) - 1; // gives exact rate

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

            var growthFactor = Math.Exp(_interestRate * _timeStep); // Hull 12.6

            foreach (var state in _result)
                state.Data.ProbabilityHeads = (growthFactor - _downFactor) / (_upFactor - _downFactor); // Hull 12.5


            return this;
        }

        public ITriMatBuilderWithDelta WithPremium(OptionExerciseType exerciseType)
        {
            _exerciseType = exerciseType;
            _optionPricingStrategy = GetOptionPricingStrategy(exerciseType);
            for (int step = _result.matrix.Length - 1; step >= 0; step--)
                for (int downMoves = _result.matrix[step].Length - 1; downMoves >= 0; downMoves--)
                {
                    var node = _result.matrix[step][downMoves];
                    State? heads = node.Heads?.Data;
                    State? tails = node.Tails?.Data;

                    if (node?.Data?.PayOff is null) throw new NullReferenceException($"Payoff is not set");

                    if (step == _result.matrix.Length - 1 || exerciseType == OptionExerciseType.American)
                    {
                        node.Data.OptionValue = _optionPricingStrategy(node.Data, heads, tails);
                        continue;
                    }

                    node.Data.OptionValue = node.Data.DiscountRate *
                                              ( heads.OptionValue * node.Data.ProbabilityHeads +
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

        public ITriMatBuilderWithPsuedoOptimalStoppingTime WithDelta()
        {
            for (int step = _result.matrix.Length - 2; step >= 0; step--)
                for (int downMoves = _result.matrix[step].Length - 1; downMoves >= 0; downMoves--)
                {
                    var node = _result.matrix[step][downMoves];
                    State heads = node.Heads.Data;
                    State tails = node.Tails.Data;

                    if (heads.UnderlyingValue == tails.UnderlyingValue)
                        throw new DivideByZeroException("Underlying Value of heads and tails node are the same leading to a divide by 0 error");

                    node.Data.DeltaHedging = (heads.OptionValue - tails.OptionValue)
                                            / (heads.UnderlyingValue - tails.UnderlyingValue);
                }

            return this;
        }

        public ITriMatBuilderReady WithPsuedoOptimalStoppingTime()
        {
            if (_exerciseType != OptionExerciseType.American)
                return this;

            //Diag Traverseal
            for (int timeStep = 0; timeStep < _result.matrix.Length; ++timeStep)
            {
                int optimalExEarlyPutTimeStep = int.MaxValue;
                for (int diagMoves = 0; diagMoves < _result.matrix[_result.matrix.Length -1 - timeStep].Length; ++diagMoves)
                {
                    var state = _result.matrix[timeStep + diagMoves][diagMoves].Data;
                    if (state.OptionValue == state.PayOff && optimalExEarlyPutTimeStep == int.MaxValue )
                            optimalExEarlyPutTimeStep = timeStep + diagMoves;
                    state.OptimalExerciseTime = optimalExEarlyPutTimeStep;
                }
            }

            return this;
        }

        public TriangularMatrix<TriMatNode<State>, State> Build()
        {
            return _result;
        }

    }
}
