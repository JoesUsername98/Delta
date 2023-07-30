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
  public class ExpexctedBinaryTreeEnhancerTests
  {
    [Theory]
    [InlineData(4, 3, 2, 0.25)]
    public void GenerateTreeWithExpectedValues(int So, int N, double u, double r)
    {
      //arrange 
      var d = 1 / u;
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
      new RiskNuetralProbabilityEnhancer().Enhance(tree);
      new ExpectedBinaryTreeEnhancer("UnderlyingValue").Enhance(tree);

      //assert
      foreach (var node in tree.Where(n => n.Heads != null && n.Tails != null))
      {
        Assert.Equal(node.Data.UnderlyingValue, node.Data.DiscountRate * node.Data.Expected.UnderlyingValue); //eq (2.3.5)
        Assert.Equal(Math.Round(node.Data.UnderlyingValue * Math.Pow(node.Data.DiscountRate, node.Time), 4),
              Math.Round(node.Data.Expected.UnderlyingValue * Math.Pow(node.Data.DiscountRate, node.Time + 1), 4)); //eq (2.4.5)
      }
    }
  }
}
