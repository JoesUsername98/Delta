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
  public class UnderlyingValueEnhancerTests
  {
    [Theory]
    [InlineData(4, 3, 2)]
    [InlineData(4, 10, 2)]
    [InlineData(4, 2, 1.1)]
    [InlineData(4, 3, 2, 0.33)]
    [InlineData(4, 10, 2, 0.8)]
    [InlineData(4, 2, 1.1, 0.1)]
    public void GenerateTreeWithUnderlyingValue(int So, int N, double u, double d = -1)
    {
      //arrange 
      var tree = BinaryTreeFactory.CreateTree(N);
      var underlyingEnhancer = d == -1 ? new UnderlyingValueBinaryTreeEnhancer(So, u) : new UnderlyingValueBinaryTreeEnhancer(So, u, d);
      underlyingEnhancer.Enhance(tree);

      //act 
      var maxValueInTree = tree.Max(x => x.Data.UnderlyingValue);
      var minValueInTree = tree.Min(x => x.Data.UnderlyingValue);
      var exMin = d == -1 ? So * Math.Pow(u, -N) : So * Math.Pow(d, N);

      //assert
      Assert.Equal(tree.Count, Enumerable.Range(0, N + 1).Sum(i => Combinations.summedNCR(i)));
      Assert.Equal(So * Math.Pow(u, N), maxValueInTree);
      Assert.Equal(exMin, minValueInTree);
    }
  }
}
