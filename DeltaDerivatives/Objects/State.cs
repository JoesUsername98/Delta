using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Objects
{
    public class State : ExpectableState, IEquatable<State>
  { 
    public State()
    {
      Expected = new ExpectableState();
    }
    public double OptionValue { get; set; }
    public double DeltaHedging { get; set; }
    private double _probabilityHeads;
    public double ProbabilityHeads {
      get { return _probabilityHeads; }
      set {
        if (value <= 0) throw new InvalidOperationException("probabilty cannot be <= 0");
        if (value >= 1) throw new InvalidOperationException("probabilty cannot be >= 0");

        _probabilityHeads = value;
      }
    }
    public double ProbabilityTails { get { return 1 - ProbabilityHeads; } }
    public int OptimalExerciseTime { get; set; }
    public ExpectableState Expected { get; set; }
    public bool Equals(State other)
    {
      return UnderlyingValue == other.UnderlyingValue &&
              ProbabilityHeads == other.ProbabilityHeads &&
              PayOff == other.PayOff &&
              InterestRate == other.InterestRate &&
              Expected == other.Expected;
    }
    /// <summary>
    /// Get the discount rate of this node down to N = 0 (root)
    /// </summary>
    public static double GetAbsoluteDiscountRate(INode<State> node) =>
      node.
      Skip(1). // this nodes discount rate refers to the rate to get to the next node. skip.
      Select(nn => nn.Data.DiscountRate).
      Aggregate((double)1, (acc, val) => acc * val); // product of all the discout


    /// <summary>
    /// Get the absolute probability of this outcome
    /// </summary>
    public static double GetAbsoluteProb(INode<State> node)
    {
      var pathStack = new Stack<bool>(node.Path);
      var currNode = node;
      double probabiltyProduct = 1;
      while (pathStack.Count > 0)
      {
        var lastCoinToss = pathStack.Pop();
        probabiltyProduct *= lastCoinToss ? currNode.Previous.Data.ProbabilityHeads : currNode.Previous.Data.ProbabilityTails;
        currNode = node.Previous;
      }
      return probabiltyProduct;
    }
  }
}
