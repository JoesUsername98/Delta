using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Interfaces;
using DeltaDerivatives.Visitors;
using Xunit;

namespace DeltaTests.Derivatives.OptionPricing
{
  public class AmericanOptimalStoppingTimeTests
  {
    public class AmericanOptionsTests
    {
      [Theory]
      [InlineData(4, 3, 2, 5, 0.25)]
      public void OptimalStoppingTimeCall(int So, int N, double u, double k, double r)
      {
        //arrange 
        var d = 1 / u;
        var tree = BinaryTreeFactory.CreateTree(N);
        var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
        underlyingEnhancer.Enhance(tree);
        var optionExercisePolicy = OptionExerciseType.American;

        //act 
        new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
        new PayoffBinaryTreeEnhancer(OptionPayoffType.Call, k).Enhance(tree);
        new RiskNuetralProbabilityEnhancer().Enhance(tree);
        new ExpectedBinaryTreeEnhancer("PayOff").Enhance(tree);
        new OptionPriceBinaryTreeEnhancer(optionExercisePolicy).Enhance(tree);
        if (optionExercisePolicy == OptionExerciseType.American) new StoppingTimeBinaryTreeEnhancer().Enhance(tree);

        //assert
        //prevent inefficient markets => prevent insider trading
        Assert.All<INode<State>>(tree, n =>
          Assert.True(n.Data.OptimalExerciseTime <= n.Time ||
            n.Data.OptimalExerciseTime == int.MaxValue));

        //what a optimal pay off is
        Assert.All<INode<State>>(tree.Where( n => n.Data.OptionValue == n.Data.PayOff), n =>
          Assert.True(n.Time >= n.Data.OptimalExerciseTime));
      }

      [Theory]
      [InlineData(4, 2, 2, 5, 0.25)]
      public void OptimalStoppingTimePut(int So, int N, double u, double k, double r)
      {
        //arrange 
        var d = 1 / u;
        var tree = BinaryTreeFactory.CreateTree(N);
        var underlyingEnhancer = new UnderlyingValueBinaryTreeEnhancer(So, u, d);
        underlyingEnhancer.Enhance(tree);
        var optionExercisePolicy = OptionExerciseType.American;

        //act 
        new ConstantInterestRateBinaryTreeEnhancer(r).Enhance(tree);
        new PayoffBinaryTreeEnhancer(OptionPayoffType.Put, k).Enhance(tree);
        new RiskNuetralProbabilityEnhancer().Enhance(tree);
        new ExpectedBinaryTreeEnhancer("PayOff").Enhance(tree);
        new OptionPriceBinaryTreeEnhancer(optionExercisePolicy).Enhance(tree);
        if(optionExercisePolicy==OptionExerciseType.American) new StoppingTimeBinaryTreeEnhancer().Enhance(tree);

        //assert

        //fig 4.3.1 where int So = 4, int N = 2, double u = 2, double k = 5, double r = 0.25
        Assert.Equal(1, tree.GetAt(new bool[] { false, false }).Data.OptimalExerciseTime);
        Assert.Equal(1, tree.GetAt(new bool[] { false }).Data.OptimalExerciseTime);
        Assert.Equal(1, tree.GetAt(new bool[] { false, true }).Data.OptimalExerciseTime);
        Assert.Equal(int.MaxValue, tree.GetAt(new bool[] {}).Data.OptimalExerciseTime);
        Assert.Equal(2, tree.GetAt(new bool[] { true, false }).Data.OptimalExerciseTime);
        Assert.Equal(int.MaxValue, tree.GetAt(new bool[] { true }).Data.OptimalExerciseTime);
        Assert.Equal(int.MaxValue, tree.GetAt(new bool[] { true,true }).Data.OptimalExerciseTime);

        //prevent inefficient markets => prevent insider trading 
        Assert.All<INode<State>>(tree, n => 
          Assert.True(n.Data.OptimalExerciseTime <= n.Time || 
            n.Data.OptimalExerciseTime == int.MaxValue));

        //what a optimal pay off is
        Assert.All<INode<State>>(tree.Where(n => n.Data.OptionValue == n.Data.PayOff), n =>
         Assert.True(n.Time >= n.Data.OptimalExerciseTime));
      }
    }
  }
}
