using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
    public class ConstantInterestRateBinaryTreeEnhancer : IBinaryTreeEnhancer
  {
    private readonly double _constantInterestRate;
    public ConstantInterestRateBinaryTreeEnhancer(double constantInterestRate)
    {
      _constantInterestRate = constantInterestRate;
    }
    public void Enhance(BinaryTree<Node<State>, State> subject)
    {
      foreach (var node in subject)
      {
        node.Data.InterestRate = _constantInterestRate; 
      }
      subject.ConstantInterestRate = _constantInterestRate;
    }
  }
}
