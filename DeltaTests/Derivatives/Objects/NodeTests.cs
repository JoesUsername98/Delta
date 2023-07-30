using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace DeltaTests.Derivatives.Objects
{
  public class NodeTests
  {
    [Theory]
    [InlineData(1, 2, 3, 4, 5, 6, 7)]
    [InlineData(2, 2, 3, 4, 7, 6, 7)]
    [InlineData(-1, 0, 3, 9, 5, 6, 7)]
    [InlineData(19, 2, 3, 4, 18, 6, 7)]
    public void TestMaxInPath(int data1, int data2, int data3, int data4, int data5, int data6, int data7)
    {
      //arrange
      var data = new List<int>() { data1, data2, data3, data4, data5, data6, data7 };
      var bt = BinaryTreeTestFactory.GenerateHeadsOnlyTree(data);
      //act
      var testNode = bt.GetAt(new bool[] { true, true, true, true, true, true });
      var maxDataInTree = testNode.Max(x => x.Data);
      var maxInPath = testNode.Where(x => x.Data == maxDataInTree);
      //assert
      Assert.Equal(data.Max(), maxInPath.First().Data);
      Assert.Equal(data.Count(d => d == data.Max()), maxInPath.Count());
    }

    [Theory]
    [InlineData(1, 2, 3, 4, 5, 6, 7)]
    [InlineData(2, 2, 3, 4, 7, 6, 7)]
    [InlineData(-1, 0, 3, 9, 5, 6, 7)]
    [InlineData(19, 2, 3, 4, 18, 6, 7)]
    public void TestMinInPath(int data1, int data2, int data3, int data4, int data5, int data6, int data7)
    {
      //arrange
      var data = new List<int>() { data1, data2, data3, data4, data5, data6, data7 };
      var bt = BinaryTreeTestFactory.GenerateHeadsOnlyTree(data);
      //act
      var testNode = bt.GetAt(new bool[] { true, true, true, true, true, true });
      var minDataInTree = testNode.Min(x => x.Data);
      var minInPath = testNode.Where(x => x.Data == minDataInTree); ;
      //assert
      Assert.Equal(data.Min(), minInPath.First().Data);
      Assert.Equal(data.Count(d => d == data.Min()), minInPath.Count());
    }
  }
}
