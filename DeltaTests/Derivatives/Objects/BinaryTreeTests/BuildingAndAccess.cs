using System;
using DeltaTests.Derivatives.Objects;
using Xunit;

namespace DeltaTests.Derivatives.Objects.BinaryTreeTests
{
  public class BuildingAndAccess
  {
    [Fact]
    public void InsertAndGet()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();

      //act and assert
      Assert.Equal("root",bt.GetAt(new bool[] { }).Data);
      Assert.Equal("h", bt.GetAt(new bool[] { true }).Data);
      Assert.Equal("t", bt.GetAt(new bool[] { false }).Data);
      Assert.Equal("hh", bt.GetAt(new bool[] { true, true }).Data);
      Assert.Equal("ht", bt.GetAt(new bool[] { true, false }).Data);
      Assert.Equal("tt", bt.GetAt(new bool[] { false, false }).Data);
      Assert.Equal("th", bt.GetAt(new bool[] { false, true }).Data);
      Assert.Equal(7, bt.Count);
      Assert.Equal(2, bt.TimeSteps);
    }
    [Fact]
    public void GetAnItemThatDoesntExist()
    {
      //arange
      var nonExistentPath = new bool[] { false, true, false };
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();

      //act and assert
      var ex = Assert.Throws<ArgumentException>(() => bt.GetAt(nonExistentPath));
      Assert.Equal(ex.ParamName, "path");

      //arange
      nonExistentPath = new bool[] { false, true, false, false};
      bt = BinaryTreeTestFactory.GenerateTimeTwoTree();

      //act and assert
      ex = Assert.Throws<ArgumentException>(() => bt.GetAt(nonExistentPath));
      Assert.Equal(ex.ParamName, "path");
    }
  }
}
