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
      //act
      var btClone = (BinaryTree<Node<string>, string>)bt.Clone();
      //assert meta data correct
      Assert.Equal(bt.Count, btClone.Count);
      Assert.Equal(bt.Time, btClone.Time);
    }

  }
}
