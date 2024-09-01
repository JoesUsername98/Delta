using Xunit;

namespace DeltaTests.Derivatives.Objects.BinaryTreeTests
{
  public class RemoveNode
  {

    [Fact]
    public void RemoveLeafNodes()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();

      //act
      bt.Remove(bt.GetAt(new bool[] { true, true }));

      //assert
      Assert.Equal(6, bt.Count);
      Assert.Equal(2, bt.TimeSteps);

      //rearrange and react
      bt.Remove(bt.GetAt(new bool[] { false, false }));

      //reassert
      Assert.Equal(5, bt.Count);
      Assert.Equal(2, bt.TimeSteps);

      //rearrange and react
      bt.Remove(bt.GetAt(new bool[] { false, true }));

      //reassert
      Assert.Equal(4, bt.Count);
      Assert.Equal(2, bt.TimeSteps);

      //rearrange and react
      bt.Remove(bt.GetAt(new bool[] { true, false }));

      //reassert
      Assert.Equal(3, bt.Count);
      Assert.Equal(1, bt.TimeSteps);
    }

    [Fact]
    public void RemoveBranchNodes()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();

      //act
      bt.Remove(bt.GetAt(new bool[] { false }));// also deletes th and tt

      //assert
      Assert.Equal(4, bt.Count);
      Assert.Equal(2, bt.TimeSteps);

      //rearrange and react
      bt.Remove(bt.GetAt(new bool[] { true }));// also deletes hh and ht

      //reassert
      Assert.Equal(1, bt.Count);
      Assert.Equal(0, bt.TimeSteps);
    }
    [Fact]
    public void RemoveRoot()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();
      //act
      bt.Remove(bt.GetAt(new bool[] { }));

      //assert
      Assert.Equal(0, bt.Count);
      Assert.Equal(-1, bt.TimeSteps);
    }

  }
}
