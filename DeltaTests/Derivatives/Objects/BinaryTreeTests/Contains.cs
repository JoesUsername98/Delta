using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DeltaDerivatives.Objects;
using Xunit;

namespace DeltaTests.Derivatives.Objects.BinaryTreeTests
{
  public class Contains
  {
    [Fact]
    public void DoesContain()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();
      
      //act and assert
      Assert.Equal(true, bt.Contains(bt.GetAt(new bool[] {})));
    }
    [Fact]
    public void DoesNotContain()
    {
      //arrange
      var bt = BinaryTreeTestFactory.GenerateTimeTwoTree();

      //act and assert
      Assert.Equal(false, bt.Contains(new Node<string>("not in tree", new bool[] { })));
    }
  }
}
