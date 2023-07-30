using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Visitors;
using Xunit;

namespace DeltaTests.Derivatives.Factory.Enhancers
{
  public class PayOffBinaryTreeEnhancerTests
  {
    [Theory]
    [InlineData(4, 3, 2, 5, 0.25)]
    public void GenerateTreeWithCallPayoff(int So, int N, double u, double k, double r)
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
      new ExpectedBinaryTreeEnhancer("UnderlyingValue").Enhance(tree);

      //assert
      foreach (var node in tree.Where(n => n.Heads != null && n.Tails != null))
      {
        Assert.Equal(node.Data.UnderlyingValue - k, node.Data.PayOff);
      }
    }
    [Theory]
    [InlineData(4, 3, 2, 5, 0.25)]
    public void GenerateTreeWithPutPayoff(int So, int N, double u, double k, double r)
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
      new ExpectedBinaryTreeEnhancer("UnderlyingValue").Enhance(tree);

      //assert
      foreach (var node in tree.Where(n => n.Heads != null && n.Tails != null))
      {
        Assert.Equal(k - node.Data.UnderlyingValue, node.Data.PayOff);
      }
    }
  }
}
