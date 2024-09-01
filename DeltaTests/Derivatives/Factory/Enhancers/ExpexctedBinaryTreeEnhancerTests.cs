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
    [InlineData(400, 3, 2, 0.25)]
    public void GenerateTreeWithExpectedValues(int So, int N, double u, double r)
    {
      //arrange 
      var d = 1 / u;
      var tree = BinaryTreeFactory.CreateTree(N, 1D );
      var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
      new RiskNuetralProbabilityEnhancer().Enhance(tree);
      new ExpectedBinaryTreeEnhancer("UnderlyingValue").Enhance(tree);

      //This tolerance is so loose as e^rdt for discounting doues for for large dt. Same goes for how we do underlying value. 
      double relativeTol = 0.03;//% 
      //assert
      foreach (var node in tree.Where(n => n.Heads != null && n.Tails != null))
      {
        Assert.True( Math.Abs(node.Data.UnderlyingValue - node.Data.DiscountRate * node.Data.Expected.UnderlyingValue ) / node.Data.UnderlyingValue < relativeTol); //eq (2.3.5)
        
        double pvPath = node.Data.UnderlyingValue * Math.Pow(node.Data.DiscountRate, node.TimeStep);
        double pvExpectedPath = node.Data.Expected.UnderlyingValue * Math.Pow(node.Data.DiscountRate, node.TimeStep + 1);
        Assert.True( Math.Abs(pvPath - pvExpectedPath) / pvPath < relativeTol); //eq (2.4.5)
      }
    }
  }
}
