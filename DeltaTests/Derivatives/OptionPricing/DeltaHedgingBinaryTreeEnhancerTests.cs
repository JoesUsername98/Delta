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
  public class DeltaHedgingBinaryTreeEnhancerTests
  {
    [Theory]
    [InlineData(4, 2, 2, 5, 0.25)] //Example 4.2.1 Exercise 4.2
    public void GenerateTreeWithAmericanPutDeltaHedging(int So, int N, double u, double k, double r)
    {
      //act and arrange 
      var tree = BinaryTreeFactory.CreateTree(N, 1D,
            new UnderlyingValueBinaryTreeEnhancer(So, u),
            new ConstantInterestRateBinaryTreeEnhancer(r),
            new PayoffBinaryTreeEnhancer(OptionPayoffType.Put, k),
            new RiskNuetralProbabilityEnhancer(),
            new ExpectedBinaryTreeEnhancer("PayOff"),
            new OptionPriceBinaryTreeEnhancer(OptionExerciseType.American),
            new DeltaHedgingBinaryTreeEnhancer());

      //assert //Example 4.2.1 Exercise 4.2
      Assert.Equal(-0.433, Math.Round(tree.GetAt(new bool[] { }).Data.DeltaHedging, 3));
      Assert.Equal(Math.Round((double)-1 / 12, 3), Math.Round(tree.GetAt(new bool[] { true }).Data.DeltaHedging, 3));
      Assert.Equal(-1, Math.Round(tree.GetAt(new bool[] { false }).Data.DeltaHedging, 3));
    }
  }
}

