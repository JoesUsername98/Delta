using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
    public class UnderlyingValueBinaryTreeEnhancer : IBinaryTreeEnhancer
  {
    private readonly double _initialPrice;
    private readonly double _upFactor;
    private readonly double _downFactor;
    public UnderlyingValueBinaryTreeEnhancer(double initialPrice, double upFactor)
    {
      if (initialPrice < 0) throw new ArgumentException("initialPrice cannot be 0", "initialPrice");
      if (upFactor <= 0) throw new ArgumentException("upFactor cannot be 0 or less", "upFactor");
      if (upFactor < 1/upFactor) throw new ArgumentException("upFactor cannot be less than downFactor");

      _initialPrice = initialPrice;
      _upFactor = upFactor;
      _downFactor = 1/upFactor;
    }
    public UnderlyingValueBinaryTreeEnhancer(double initialPrice, double upFactor, double downFactor)
    {
      if (initialPrice < 0) throw new ArgumentException("initialPrice cannot be 0", "initialPrice");
      if (upFactor <= 0) throw new ArgumentException("upFactor cannot be 0 or less", "upFactor");
      if (downFactor <= 0) throw new ArgumentException("downFactor cannot be 0 or less", "downFactor");
      if (upFactor < downFactor) throw new ArgumentException("upFactor cannot be less than downFactor");

      _initialPrice = initialPrice;
      _upFactor = upFactor;
      _downFactor = downFactor;
    }
    public void Enhance(BinaryTree<Node<State>, State> subject)
    {
      foreach (var node in subject)
      {
        var state = new State();
        var noOfHeads = node.Path.Count(p => p);
        var noOfTails = node.Time - noOfHeads;
        node.Data.UnderlyingValue = _initialPrice * Math.Pow(_upFactor, noOfHeads) * Math.Pow(_downFactor, noOfTails);
      }

      subject.ConstantUpFactor = _upFactor;
      subject.ConstantDownFactor = _downFactor;
    }
  }
}
