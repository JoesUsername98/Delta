﻿namespace DeltaDerivatives.Objects.Interfaces
{
    public interface IBinaryTree<N,T> : ICloneable, ICollection<N>, IEnumerable<N> where N : INode<T> where T : IEquatable<T>
    {
    double? ConstantUpFactor { get; set; }
    double? ConstantDownFactor { get; set; }
    double? ConstantInterestRate { get; set; }
    int Time  { get; }
    N GetAt(bool[] path);
  }
}