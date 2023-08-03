using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Factory
{
  public static class BinaryTreeFactory
  {
    public static BinaryTree<Node<State>,State> CreateTree(int time)
    {
      if (time < 0) throw new ArgumentException("time cannot be less than 0", "time");

      var bt = new BinaryTree<Node<State>, State>(new Node<State>(new State(), new bool[] { }));
      for (int currTime = 1; currTime <= time; currTime++)
      {
        var inputParams = Combinations.GenerateParams(new bool[] { true, false }, currTime);
        foreach (IEnumerable<bool> path in Combinations.Parameters(inputParams))
        {
          bt.Add(new Node<State>(new State(), path.ToArray()));
        }
      }
      return bt;
    }

  }
}
