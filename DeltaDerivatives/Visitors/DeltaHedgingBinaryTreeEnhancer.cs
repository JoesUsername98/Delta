using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
    public class DeltaHedgingBinaryTreeEnhancer : IBinaryTreeEnhancer
  {
    public DeltaHedgingBinaryTreeEnhancer()
    {
    }
    public void Enhance(BinaryTree<Node<State>, State> subject)
    {
      foreach (var node in subject.Where(n => n.Heads != null && n.Tails != null))
      {
        if (node.Heads.Data.UnderlyingValue == node.Tails.Data.UnderlyingValue)
          throw new DivideByZeroException("Underlying Value of heads and tails node are the same leading to a divide by 0 error");

        node.Data.DeltaHedging = (node.Heads.Data.OptionValue - node.Tails.Data.OptionValue)
                                /(node.Heads.Data.UnderlyingValue - node.Tails.Data.UnderlyingValue);
      }
    }
  }
}
