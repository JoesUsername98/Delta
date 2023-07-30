namespace DeltaDerivatives.Objects.Interfaces
{
    public interface IBinaryTree<T> : ICloneable, ICollection<Node<T>>, IEnumerable<Node<T>> where T : IEquatable<T>
  {
    double? ConstantUpFactor { get; set; }
    double? ConstantDownFactor { get; set; }
    double? ConstantInterestRate { get; set; }
    int Time  { get; }
    Node<T> GetAt(bool[] path);
  }
}