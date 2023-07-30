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

namespace DeltaTests.Derivatives.Factory
{
  /// <summary>
  /// Responsible for generating trees and generating 
  /// </summary>
  public class BinaryTreeFactoryTests
  {
    [Theory]
    [InlineData(3)]
    [InlineData(10)]
    [InlineData(2)]
    [InlineData(3)]
    [InlineData(10)]
    [InlineData(2)]
    public void GenerateEmptyTree(int N)
    {
      //act and arrange 
      var tree = BinaryTreeFactory.CreateTree(N);

      //assert
      Assert.Equal(tree.Count, Enumerable.Range(0, N + 1).Sum(i => Combinations.summedNCR(i)));
    }
  }
}
