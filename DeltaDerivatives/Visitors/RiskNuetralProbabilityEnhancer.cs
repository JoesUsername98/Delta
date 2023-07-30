using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
    public class RiskNuetralProbabilityEnhancer : IBinaryTreeEnhancer
  {
    public RiskNuetralProbabilityEnhancer()
    {
        
    }
    public void Enhance(BinaryTree<State> subject)
    {
      if (!subject.ConstantUpFactor.HasValue)   throw new ArgumentNullException("ConstantUpFactor");
      if (!subject.ConstantDownFactor.HasValue) throw new ArgumentNullException("ConstantDownFactor");
      
      var u = subject.ConstantUpFactor.Value;
      var d = subject.ConstantDownFactor.Value;

      foreach (var node in subject)
      {
        if (u <= 1 + node.Data.InterestRate) throw new InvalidOperationException("u > 1 + r to prevent arbitrage");
        if (1 + node.Data.InterestRate <= d) throw new InvalidOperationException("1 + r > d to prevent arbitrage");
        if (d <= 0) throw new InvalidOperationException("d > 0 to prevent arbitrage");

        node.Data.ProbabilityHeads = (1 + node.Data.InterestRate - d) / (u - d);
      }
    }
  }
}
