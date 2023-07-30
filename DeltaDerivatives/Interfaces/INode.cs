namespace DeltaDerivatives.Objects.Interfaces
{
    public interface INode<T> : ICloneable, IEnumerable<INode<T>>, IEquatable<INode<T>> where T : IEquatable<T> 
  {
    T Data { get; }
    INode<T> Heads { get; set; }
    bool[] Path { get; }
    INode<T> Previous { get; set; }
    INode<T> Tails { get; set; }
    int Time { get; }
    int CountSubsequentNodes(INode<T> node);
    int CountTime(INode<T> node);
    INode<T> GetNext(bool isHeads);
    void AddNext(INode<T> nextItem, bool isHeads);
    IEnumerator<INode<T>> GetForesightEnumerator();
  }
}