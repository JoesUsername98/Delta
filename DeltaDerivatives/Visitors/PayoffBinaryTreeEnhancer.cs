using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
    public class PayoffBinaryTreeEnhancer : IBinaryTreeEnhancer
  {
    private readonly Func<INode<State>, double, double> _payoffMethod;
    private readonly double _strikePrice;

    public PayoffBinaryTreeEnhancer(OptionPayoffType optionType, double strikePrice)
    {
      _payoffMethod = GetPayoffStrategy(optionType);
      _strikePrice = strikePrice;
    }

    public void Enhance(BinaryTree<Node<State>, State> subject)
    {
      foreach (var node in subject)
      {
        node.Data.PayOff = _payoffMethod(node, _strikePrice);
      }
    }

    private Func<INode<State>, double, double> GetPayoffStrategy(OptionPayoffType type)
    {
      switch (type)
      {
        case OptionPayoffType.Call:
          return (x, K) => x.Data.UnderlyingValue - K;
        case OptionPayoffType.Put:
          return (x, K) => K - x.Data.UnderlyingValue;
        default:
          throw new ArgumentException($"No payoff strategy for type {type}", "type");
      }
    }
  }
}
