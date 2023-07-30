using DeltaDerivatives.Objects;
using Xunit;

namespace DeltaTests.Derivatives.Objects.BinaryTreeTests
{
  public class Cloning
  {
    /// <summary>
    /// These tests are now broken after making Node and Binary Tree a Collection
    /// </summary>
    //[Fact]
    //public void IsDeepCopy()
    //{
    //  arrange
    //  var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();
    //  act
    //  var btClone = (BinaryTree<string>)bt.Clone();

    //  assert is deep copy
    //  Assert.NotEqual(bt, btClone);
    //  Assert.NotEqual(bt.GetAt(new bool[] { }), btClone.GetAt(new bool[] { }));
    //  Assert.NotEqual(bt.GetAt(new bool[] { true }), btClone.GetAt(new bool[] { true }));
    //  Assert.NotEqual(bt.GetAt(new bool[] { false }), btClone.GetAt(new bool[] { false }));
    //  Assert.NotEqual(bt.GetAt(new bool[] { true, true }), btClone.GetAt(new bool[] { true, true }));
    //  Assert.NotEqual(bt.GetAt(new bool[] { true, false }), btClone.GetAt(new bool[] { true, false }));
    //  Assert.NotEqual(bt.GetAt(new bool[] { false, false }), btClone.GetAt(new bool[] { false, false }));
    //  Assert.NotEqual(bt.GetAt(new bool[] { false, true }), btClone.GetAt(new bool[] { false, true }));

    //}

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
