﻿using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaDerivatives.Visitors
{
  public class StoppingTimeBinaryTreeEnhancer : IBinaryTreeEnhancer
  {
    private readonly OptionExerciseType _exerciseType;
    
    // TODO DEPRECATE UNSAFE USAGE
    public StoppingTimeBinaryTreeEnhancer()
    {
      _exerciseType = OptionExerciseType.American;
    }
    public StoppingTimeBinaryTreeEnhancer(OptionExerciseType exType )
    {
       _exerciseType = exType;
    }
    public void Enhance(BinaryTree<Node<State>, State> subject)
    {
      if (_exerciseType != OptionExerciseType.American) 
        return;

      //Expiration time nodes. 
      foreach (var node in subject.Where(n => n.Time == subject.Time))
      {
        var nodesInPathWhereShouldExercise = node.Where(n => n.Data.OptionValue == n.Data.PayOff).OrderBy(n => n.Time).FirstOrDefault();

        //No optimal optimal exercise 
        if (nodesInPathWhereShouldExercise is null)
        {
          node.Data.OptimalExerciseTime = node.Data.OptionValue > 0 ? node.Time : int.MaxValue;
          continue;
        }
        node.Data.OptimalExerciseTime = nodesInPathWhereShouldExercise.Time;
      }

      foreach (var node in subject.Where(n => n.Time < subject.Time).OrderByDescending(n => n.Time))
      {
        //if exercise time nodes find the min stopping time to be this node or prior in path,
        //set stopping time to that minimum, else set to max int (do not exercise)
        var minStopingTime = Math.Min(node.Heads.Data.OptimalExerciseTime ?? int.MaxValue, node.Tails.Data.OptimalExerciseTime ?? int.MaxValue);
        node.Data.OptimalExerciseTime = minStopingTime <= node.Time ? minStopingTime : int.MaxValue;
        
      }
    }
  }
}
