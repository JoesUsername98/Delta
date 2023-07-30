using DeltaDerivatives.Objects.Interfaces;
using DeltaDerivatives.Objects.Iterators;
using System.Collections;
using System.Runtime.Serialization;

namespace DeltaDerivatives.Objects
{
    [DataContract]
  public class Node<T> : INode<T> where T : IEquatable<T>
  {
    public Node(T data, bool[] path)
    {
      Data = data;
      Path = path;
    }
    [DataMember]
    public T Data { get; set; }
    [DataMember]
    public bool[] Path { get; private set; }
    [DataMember]
    public int Time { get { return Path.Length; } }
    [DataMember]
    public INode<T> Previous { get; set; }
    [DataMember]
    public INode<T> Heads { get; set; }
    [DataMember]
    public INode<T> Tails { get; set; }
    public INode<T> GetNext(bool isHeads)
    {
      return isHeads ? Heads : Tails;
    }
    public void AddNext(INode<T> newNode, bool isNextTossIsHeads)
    {
      newNode.Previous = this;

      if (isNextTossIsHeads)
      {
        if (Heads != null) throw new ArgumentException("Cannot overwrite an existing head node");
        Heads = newNode;
        return;
      }

      if (Tails != null) throw new ArgumentException("Cannot overwrite an existing tails node");
      Tails = newNode;
    }
    public int CountSubsequentNodes(INode<T> node)
    {
      if (node is null) return 0;

      var subTree = new BinaryTree<INode<T>, T>(node);
      return subTree.Count;
    }
    public int CountTime(INode<T> node)
    {
      if (node == null) return -1;

      var subTree = new BinaryTree<INode<T>, T>(node);
      return subTree.Time;
    }
    public object Clone()
    {
      var copiedNode = new Node<T>(this.Data, this.Path);
      if (Heads != null)
      {
        copiedNode.Heads = (Node<T>)this.Heads.Clone();
        copiedNode.Heads.Previous = copiedNode;
      }
      if (Tails != null)
      {
        copiedNode.Tails = (Node<T>)this.Tails.Clone();
        copiedNode.Tails.Previous = copiedNode;
      }
      return copiedNode;
    }
    /// <summary>
    /// Returns only the history of the node.
    /// </summary>
    /// <returns></returns>
    public IEnumerator<INode<T>> GetEnumerator() => new NodeReverseOrderIterator<T>(this);
    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    
    /// <summary>
    /// Returns the node with foresight. For Tree Traversal
    /// </summary>
    /// <returns></returns>
    public IEnumerator<INode<T>> GetForesightEnumerator() => new NodeInOrderIterator<T>(this);


    public bool Equals(INode<T> other)
    {
      return this.Data.Equals(other.Data) && this.Path.Equals(other.Path);
    }

  }
}
