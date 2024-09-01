using System;
using System.Collections.Generic;
using System.Linq;
using DeltaDerivatives.Builders;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using Xunit;

namespace DeltaTests.Derivatives.Pricing
{
    public class TimeSteps
    {
        [Fact]
        public void ConstantPriceWithVaringTimeSteps()
        {
            int So = 4;
            double M = 1D;
            double u = 2;
            double k = 5;
            double r = /*0.25*/ 0.1;
            int stepsOne = 1;
            int stepsTwo = 2;
            int steps20 = 20;
            int steps100 = 100;
            int steps500 = 500;
            int steps1000 = 1000;

            //arrange 

            var tMat1 = new TriangularMatrixBuilder(stepsOne, M/stepsOne )
                            .WithUnderlyingValueAndUpFactor(So, u)
                            .WithInterestRate(r)
                            .WithPayoff(OptionPayoffType.Call, k)
                            .WithRiskNuetralProb()
                            .WithPremium(OptionExerciseType.European)
                            .WithDelta()
                            .Build();
            var priceOne = tMat1[0, 0].Data.OptionValue;
            var rate1 = getRate(tMat1.matrix);

            var tMat2 = new TriangularMatrixBuilder(stepsTwo, M / stepsTwo )
                .WithUnderlyingValueAndUpFactor(So, u)
                .WithInterestRate(r)
                .WithPayoff(OptionPayoffType.Call, k)
                .WithRiskNuetralProb()
                .WithPremium(OptionExerciseType.European)
                .WithDelta()
                .Build();

            var priceTwo = tMat2[0, 0].Data.OptionValue;
            var rate2 = getRate(tMat2.matrix);

            var tMat20 = new TriangularMatrixBuilder(steps20, M / steps20 )
                .WithUnderlyingValueAndUpFactor(So, u)
                .WithInterestRate(r)
                .WithPayoff(OptionPayoffType.Call, k)
                .WithRiskNuetralProb()
                .WithPremium(OptionExerciseType.European)
                .WithDelta()
                .Build();

            var price20 = tMat20[0, 0].Data.OptionValue;
            var rate20 = getRate(tMat20.matrix);

            var tMat100 = new TriangularMatrixBuilder(steps100, M / steps100)
                .WithUnderlyingValueAndUpFactor(So, u)
                .WithInterestRate(r)
                .WithPayoff(OptionPayoffType.Call, k)
                .WithRiskNuetralProb()
                .WithPremium(OptionExerciseType.European)
                .WithDelta()
                .Build();

            var price100 = tMat100[0, 0].Data.OptionValue;
            var rate100 = getRate(tMat100.matrix);

            var tMat500 = new TriangularMatrixBuilder(steps500, M / steps500)
                .WithUnderlyingValueAndUpFactor(So, u)
                .WithInterestRate(r)
                .WithPayoff(OptionPayoffType.Call, k)
                .WithRiskNuetralProb()
                .WithPremium(OptionExerciseType.European)
                .WithDelta()
                .Build();

            var price500 = tMat500[0, 0].Data.OptionValue;
            var rate500 = getRate(tMat500.matrix);

            var tMat1000 = new TriangularMatrixBuilder(steps1000, M / steps1000)
                .WithUnderlyingValueAndUpFactor(So, u)
                .WithInterestRate(r)
                .WithPayoff(OptionPayoffType.Call, k)
                .WithRiskNuetralProb()
                .WithPremium(OptionExerciseType.European)
                .WithDelta()
                .Build();

            var price1000 = tMat1000[0, 0].Data.OptionValue;
            var rate1000 = getRate(tMat1000.matrix);
            var h1000 = getAllHeads(tMat1000.matrix);
            //assert
        }
        double getRate(TriMatNode<State>[][] mat)
        {
            var recalcInterestRate = 1D;
            for (int steps = 1; steps < mat.Length; steps++)
                recalcInterestRate *= (1D + mat[steps][0].Data.InterestRate);
            return recalcInterestRate;
        }
        double getAllHeads(TriMatNode<State>[][] mat)
        {
            return mat[mat.Length-1][0].Data.UnderlyingValue;
        }
        //[Theory]
        //[InlineData(1)]
        //public void GenerateTreeWithEuropeanCallPrice(int stepToUse)
        //{
        //    //arrange 
        //    int So = 4;
        //    double M = 1D;
        //    double u = 2;
        //    double k = 5;
        //    double r = /*0.25*/ 0.1;
        //    //act 
        //    var tMat = new TriangularMatrixBuilder(stepToUse, M / stepToUse)
        //                                .WithUnderlyingValue(So, u)
        //                                .WithInterestRate(r)
        //                                .WithPayoff(OptionPayoffType.Call, k)
        //                                .WithRiskNuetralProb()
        //                                .WithPremium(OptionExerciseType.European)
        //                                .WithDelta()
        //                                .Build();

        //    //assert
        //    //Theorem 2.4.7 Risk-nuetral pricing formula 
        //    //The Discounted price of a derivative security is a martingale under risk nuetral pricing. 
        //    // Vn / (1+r)^n = En ( Vn+1 / (1+r)^(n+1) )   ---   (2.4.12)
        //    var expectedOptionPriceValueAtEachTime = new Dictionary<int, double>();
        //    var discountedExpectedOptionPrice = 1D;
        //    for (int step = tMat.matrix.Length - 1; step >= 0; step--)
        //    {
        //        for (int downMoves = tMat.matrix[step].Length - 1; downMoves >= 0; downMoves--)
        //        {
        //            var thisState = tMat[step, downMoves].Data;
        //            discountedExpectedOptionPrice += thisState.OptionValue * getDiscountToTimeZero(tMat, step) * State.GetAbsoluteProb(n);
        //        }
        //        expectedOptionPriceValueAtEachTime.Add(step, Math.Round(discountedExpectedOptionPrice, 5));
        //    }
        //    Assert.Equal(1, expectedOptionPriceValueAtEachTime.Values.Distinct().Count());
        //}
        //double getDiscountToTimeZero(TriangularMatrix<TriMatNode<State>, State> mat, int t)
        //{
        //    var df = 1D;
        //    for (int steps = 0; steps < t; steps++)
        //        df *= mat[steps,0].Data.DiscountRate;
        //    return df;
        //}
        //double getProb(TriangularMatrix<TriMatNode<State>, State> mat, int t)
        //{
        //    var recalcInterestRate = 1D;
        //    for (int steps = 0; steps < t; steps++)
        //        recalcInterestRate *= (1D + mat[steps, 0].Data.InterestRate);
        //    return recalcInterestRate;
        //}
    }
}
