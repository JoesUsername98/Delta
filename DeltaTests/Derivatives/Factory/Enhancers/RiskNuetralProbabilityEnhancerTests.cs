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
  public class RiskNuetralProbabilityEnhancerTests
  {
    [Theory]
    [InlineData(4, 3, 2, 0.25)]
    [InlineData(4, 10, 2, 0.25)]
    [InlineData(4, 3, 2, 0.25, 0.33)]
    [InlineData(4, 10, 2, 0.25, 0.8)]
    public void GenerateTreeWithUnderlyingAndRiskNuetralProbs(int So, int N, double u, double r, double d = -1)
    {
      //arrange 
      d = d == -1 ? 1 / u : d;
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
      new RiskNuetralProbabilityEnhancer().Enhance(tree);

      //assert
      foreach (var node in tree) Assert.Equal(r, node.Data.InterestRate);
      foreach (var node in tree) Assert.Equal(1, node.Data.ProbabilityHeads + node.Data.ProbabilityTails);
      foreach (var node in tree) { Assert.True(node.Data.ProbabilityHeads > 0); Assert.True(node.Data.ProbabilityTails > 0); }
    }
  }
}
