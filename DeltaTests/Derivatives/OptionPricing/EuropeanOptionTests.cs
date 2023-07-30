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

namespace DeltaTests.Derivatives.Pricing
{
  public class EuropeanOptionTests
  {
    [Theory]
    [InlineData(4, 3, 2, 5, 0.25)]
    public void GenerateTreeWithEuropeanCallPrice(int So, int N, double u, double k, double r)
    {
      //arrange 
      var d = 1 / u;
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
      new PayoffBinaryTreeEnhancer(OptionPayoffType.Call, k).Enhance(tree);
      new RiskNuetralProbabilityEnhancer().Enhance(tree);
      new ExpectedBinaryTreeEnhancer("PayOff").Enhance(tree);
      new OptionPriceBinaryTreeEnhancer(OptionExerciseType.European).Enhance(tree);

      //assert
      //Theorem 2.4.7 Risk-nuetral pricing formula 
      //The Discounted price of a derivative security is a martingale under risk nuetral pricing. 
      // Vn / (1+r)^n = En ( Vn+1 / (1+r)^(n+1) )   ---   (2.4.12)
      var expectedOptionPriceValueAtEachTime = new Dictionary<int, double>();
      for (int thisTime = tree.Time; thisTime >= 0; thisTime--)
      {
        var discountedExpectedOptionPrice
          = tree.Where(n => n.Time == thisTime)
                 .Sum(n => n.Data.OptionValue * State.GetAbsoluteDiscountRate(n) * State.GetAbsoluteProb(n));

        expectedOptionPriceValueAtEachTime.Add(thisTime, Math.Round(discountedExpectedOptionPrice, 5));
      }
      Assert.Equal(1, expectedOptionPriceValueAtEachTime.Values.Distinct().Count());
    }

    [Theory]
    [InlineData(4, 3, 2, 5, 0.25)]
    public void GenerateTreeWithEuropeanPutPrice(int So, int N, double u, double k, double r)
    {
      //arrange 
      var d = 1 / u;
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
      new PayoffBinaryTreeEnhancer(OptionPayoffType.Put, k).Enhance(tree);
      new RiskNuetralProbabilityEnhancer().Enhance(tree);
      new ExpectedBinaryTreeEnhancer("PayOff").Enhance(tree);
      new OptionPriceBinaryTreeEnhancer(OptionExerciseType.European).Enhance(tree);

      //assert
      //Theorem 2.4.7 Risk-nuetral pricing formula 
      //The Discounted price of a derivative security is a martingale under risk nuetral pricing. 
      // Vn / (1+r)^n = En ( Vn+1 / (1+r)^(n+1) )   ---   (2.4.12)
      var expectedOptionPriceValueAtEachTime = new Dictionary<int, double>();
      for (int thisTime = tree.Time; thisTime >= 0; thisTime--)
      {
        var discountedExpectedOptionPrice
          = tree.Where(n => n.Time == thisTime)
                 .Sum(n => n.Data.OptionValue * State.GetAbsoluteDiscountRate(n) * State.GetAbsoluteProb(n));

        expectedOptionPriceValueAtEachTime.Add(thisTime, Math.Round(discountedExpectedOptionPrice, 5));
      }
      Assert.Equal(1, expectedOptionPriceValueAtEachTime.Values.Distinct().Count());
    }
  }
}
