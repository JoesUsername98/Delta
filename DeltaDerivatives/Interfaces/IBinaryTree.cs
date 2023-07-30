namespace DeltaDerivatives.Objects.Interfaces
{
    public interface IBinaryTree<T> : ICloneable, ICollection<INode<T>>, IEnumerable<INode<T>> where T : IEquatable<T>
  {
    double? ConstantUpFactor { get; set; }
    double? ConstantDownFactor { get; set; }
    double? ConstantInterestRate { get; set; }
    int Time  { get; }
    INode<T> GetAt(bool[] path);
  }
}