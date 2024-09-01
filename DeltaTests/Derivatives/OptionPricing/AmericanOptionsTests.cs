using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Visitors;
using Xunit;

namespace DeltaTests.Derivatives.OptionPricing
{
  public class AmericanOptionsTests
  {
    /// <summary>
    /// There is no advantage to early exercise of the american call opion 
    /// </summary>
    /// <param name="So"></param>
    /// <param name="N"></param>
    /// <param name="u"></param>
    /// <param name="k"></param>
    /// <param name="r"></param>
    [Theory]
    [InlineData(4, 3, 2, 5, 0.25)]
    public void GenerateTreeWithAmericanCallPrice(int So, int N, double u, double k, double r)
    {
      //act and arrange 
      var tree = BinaryTreeFactory.CreateTree(N,1D,
          new UnderlyingValueBinaryTreeEnhancer(So, u),
          new ConstantInterestRateBinaryTreeEnhancer(r),
          new PayoffBinaryTreeEnhancer(OptionPayoffType.Call, k),
          new RiskNuetralProbabilityEnhancer(),
          new ExpectedBinaryTreeEnhancer("PayOff"),
          new OptionPriceBinaryTreeEnhancer(OptionExerciseType.American));

      //assert
      var expectedOptionPriceValueAtEachTime = new Dictionary<int, double>();
      for (int thisTime = tree.Time; thisTime >= 0; thisTime--)
      {
        var discountedExpectedOptionPrice
          = tree.Where(n => n.TimeStep == thisTime)
                 .Sum(n => n.Data.OptionValue * State.GetAbsoluteDiscountRate(n) * State.GetAbsoluteProb(n));

        expectedOptionPriceValueAtEachTime.Add(thisTime, Math.Round(discountedExpectedOptionPrice, 5));
      }
      //since there is no advantage to early exercise. It should behave as a martingale - the same as a european call.
      Assert.Equal(1, expectedOptionPriceValueAtEachTime.Values.Distinct().Count());
    }

    [Theory]
    [InlineData(4, 6, 2, 5, 0.25)]
    public void GenerateTreeWithAmericanPutPrice(int So, int N, double u, double k, double r)
    {
      //act and arrange  
      var tree = BinaryTreeFactory.CreateTree(N, 1D,
          new UnderlyingValueBinaryTreeEnhancer(So, u),
          new ConstantInterestRateBinaryTreeEnhancer(r),
          new PayoffBinaryTreeEnhancer(OptionPayoffType.Put, k),
          new RiskNuetralProbabilityEnhancer(),
          new ExpectedBinaryTreeEnhancer("PayOff"),
          new OptionPriceBinaryTreeEnhancer(OptionExerciseType.American));

            var expectedOptionPriceValueAtEachTime = new Dictionary<int, double>();
      for (int thisTime = tree.Time; thisTime >= 0; thisTime--)
      {
        var discountedExpectedOptionPrice
          = tree.Where(n => n.TimeStep == thisTime)
                 .Sum(n => n.Data.OptionValue * State.GetAbsoluteDiscountRate(n) * State.GetAbsoluteProb(n));

        expectedOptionPriceValueAtEachTime.Add(thisTime, Math.Round(discountedExpectedOptionPrice, 5));
      }

      //assert that with time the expected value of the american option deminishes. super-martigale
      for (int i = 0; i < expectedOptionPriceValueAtEachTime.Values.Count(); i++)
       Assert.Equal(expectedOptionPriceValueAtEachTime.Values.OrderBy(s => s).ToList()[i], expectedOptionPriceValueAtEachTime.Values.ToList()[i]);
    }
  }
}
