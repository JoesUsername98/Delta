using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Factory
{
  public static class BinaryTreeFactory
  {
    public static BinaryTree<Node<State>,State> CreateTree(int timeSteps, double timeStep = 1D)
    {
      if (timeSteps < 0) throw new ArgumentException("time steps cannot be less than 0", "timeSteps");
      if (timeStep <= 0) throw new ArgumentException("time Step cannot be less than or equal to 0", "timeStep");

      var bt = new BinaryTree<Node<State>, State>(new Node<State>(new State(), new bool[] { }));
      for (int currTime = 1; currTime <= timeSteps; currTime++)
      {
        var inputParams = Combinations.GenerateParams(new bool[] { true, false }, currTime);
        foreach (IEnumerable<bool> path in Combinations.Parameters(inputParams))
        {
          bt.Add(new Node<State>(new State(), path.ToArray()));
        }
      }
      return bt;
    }
    public static BinaryTree<Node<State>, State> CreateTree(int timeSteps, double timeStep = 1D, params IBinaryTreeEnhancer[] enhancers)
    {
      var bt = CreateTree(timeSteps, timeStep);
      foreach (IBinaryTreeEnhancer enhancer in enhancers)
          enhancer.Enhance(bt);

      return bt;
    }
   }
}
