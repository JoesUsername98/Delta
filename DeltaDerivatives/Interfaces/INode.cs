namespace DeltaDerivatives.Objects.Interfaces
{
    public interface INode<T> : ICloneable, IEnumerable<INode<T>>, IEquatable<INode<T>> where T : IEquatable<T> 
  {
    T Data { get; }
    Node<T> Heads { get; set; }
    bool[] Path { get; }
    Node<T> Previous { get; set; }
    Node<T> Tails { get; set; }
    int Time { get; }
    int CountSubsequentNodes(Node<T> node);
    int CountTime(Node<T> node);
    Node<T> GetNext(bool isHeads);
    void AddNext(Node<T> nextItem, bool isHeads);
    IEnumerator<Node<T>> GetForesightEnumerator();
  }
}