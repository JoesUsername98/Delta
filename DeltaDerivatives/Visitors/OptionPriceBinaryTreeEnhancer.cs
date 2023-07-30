using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
    public class OptionPriceBinaryTreeEnhancer : IBinaryTreeEnhancer
  {
    private readonly OptionExerciseType _optionType; 
    private readonly Func<INode<State>, double> _optionPricingStrategy;

    public OptionPriceBinaryTreeEnhancer(OptionExerciseType optionType)
    {
      _optionType = optionType;
      _optionPricingStrategy = GetOptionPricingStrategy(optionType);
    }

    public void Enhance(BinaryTree<Node<State>, State> subject)
    {
      foreach (var node in subject.OrderByDescending(n => n.Time))
      {
        if (node?.Data?.PayOff is null) throw new NullReferenceException($"Payoff is null {node.Path}");

        if (node.Time == subject.Time || _optionType == OptionExerciseType.American)
        {
          node.Data.OptionValue = _optionPricingStrategy(node);
          continue;
        }

        node.Data.OptionValue = node.Data.DiscountRate *
                                  (node.Heads.Data.OptionValue * node.Data.ProbabilityHeads +
                                   node.Tails.Data.OptionValue * node.Data.ProbabilityTails);
      }
    }

    private Func<INode<State>, double> GetOptionPricingStrategy(OptionExerciseType type)
    {
      switch (type)
      {
        case OptionExerciseType.European:
          return (x) => Math.Max(x.Data.PayOff, 0);
        case OptionExerciseType.American:
          return (x) => Math.Max(x.Data.PayOff, (x.Heads is null || x.Tails is null) ? 0.0 :
                                    x.Data.DiscountRate *
                                  (x.Heads.Data.OptionValue * x.Data.ProbabilityHeads +
                                   x.Tails.Data.OptionValue * x.Data.ProbabilityTails));
        default:
          throw new ArgumentException($"No option pricing strategy for type {type}", "type");
      }
    }
  }
}
