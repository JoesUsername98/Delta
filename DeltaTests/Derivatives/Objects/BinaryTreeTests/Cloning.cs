using DeltaDerivatives.Objects;
using Xunit;

namespace DeltaTests.Derivatives.Objects.BinaryTreeTests
{
  public class Cloning
  {
    [Fact]
    public void IsMetaDataCorrect()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();
      int originalCount = bt.Count;
      int originalTime = bt.Time;

      //act
      var btClone = (BinaryTree<Node<string>, string>)bt.Clone();
      bt.Remove(bt.GetAt(new bool[] { true }));
      bt.Remove(bt.GetAt(new bool[] { false }));

      //assert meta data correct
      Assert.Equal(originalCount, btClone.Count);
      Assert.Equal(originalTime, btClone.Time);
    }
  }
}
