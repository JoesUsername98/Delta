using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Visitors;
using Xunit;

namespace DeltaTests.Derivatives.Factory.Enhancers
{
  public class ConstantInterestRateEnhancerTests
  {
    [Theory]
    [InlineData(0.25)]
    [InlineData(1)]
    [InlineData(4)]
    [InlineData(0.33)]
    [InlineData(0.8)]
    [InlineData(0.1)]
    public void GenerateTreeWithConstantInterestRate(double r)
    {
      //arrange 
      int N = 3;
      var tree = BinaryTreeFactory.CreateTree(N);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);

      //assert
      Assert.Equal(tree.Count, Enumerable.Range(0, N + 1).Sum(i => Combinations.summedNCR(i)));
      foreach (var node in tree) Assert.Equal(r, node.Data.InterestRate);
    }
    [Theory]
    [InlineData(4, 2, 1.1, 0.25)]
    ///if (u< 1 + node.Data.InterestRate) throw new InvalidOperationException("u > 1 + r to prevent arbitrage");
    public void ErrorWhenBreachingArbitrageConditions1(int So, int N, double u, double r, double d = -1)
    {
      //arrange 
      d = d == -1 ? 1 / u : d;
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);

      //assert
      var ex = Assert.Throws<InvalidOperationException>(() => new RiskNuetralProbabilityEnhancer().Enhance(tree));
      Assert.Equal(ex.Message, "u > 1 + r to prevent arbitrage");
    }

    [Theory]
    [InlineData(4, 3, 6, 4, 5.1)]
    ///if (1 + node.Data.InterestRate < d) throw new InvalidOperationException("1 + r > d to prevent arbitrage");
    public void ErrorWhenBreachingArbitrageConditions2(int So, int N, double u, double r, double d = -1)
    {
      //arrange 
      d = d == -1 ? 1 / u : d;
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);

      //assert
      var ex = Assert.Throws<InvalidOperationException>(() => new RiskNuetralProbabilityEnhancer().Enhance(tree));
      Assert.Equal(ex.Message, "1 + r > d to prevent arbitrage");
    }
  }
}
