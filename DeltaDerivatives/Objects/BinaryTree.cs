using DeltaDerivatives.Objects.Interfaces;
using System.Collections;
using System.Runtime.Serialization;

namespace DeltaDerivatives.Objects
{
    [DataContract]
  public class BinaryTree<T> : IBinaryTree<T> where T : IEquatable<T>
  {
    [DataMember]
    private INode<T> _root;
    [DataMember]
    public int Count { get; private set; }
    [DataMember]
    public int Time { get; private set; }
    public bool IsReadOnly => false;
    [DataMember]
    public double? ConstantUpFactor { get; set; }
    [DataMember]
    public double? ConstantDownFactor { get; set; }
    [DataMember]
    public double? ConstantInterestRate { get; set; }
    public BinaryTree()
    {
      _root = null;
      Count = 0;
    }
    public BinaryTree(INode<T> root)
    {
      _root = root;
      Count = this.Select(x => x).Count();
    }
    #region IEnumerable
    public IEnumerator<INode<T>> GetEnumerator()
    {
      return _root.GetForesightEnumerator();
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
      return GetEnumerator();
    }
    #endregion
    #region ICollection
    public void Add(INode<T> newItem)
    {
      if (newItem.Path.Length == 0)
      {
        _root = newItem;
        Count++;
        Time = 0;
        return;
      }
      var parentNodePath = newItem.Path.Take(newItem.Path.Length - 1);
      var pathStack = new Stack<bool>(parentNodePath.Reverse());

      var parentNode = _root;
      while (pathStack.Count > 0)
      {
        var nextTossIsHeads = pathStack.Pop();
        parentNode = nextTossIsHeads ? parentNode.Heads : parentNode.Tails;
        if (parentNode == null)
        {
          throw new ArgumentException("Could not find a parent node in this path: " + parentNodePath, "newItem.Path");
        }
      }

      parentNode.AddNext(newItem, newItem.Path.Last());
      Time = newItem.Path.Length > Time ? newItem.Path.Length : Time;
      Count++;
    }
    public bool Contains(INode<T> item)
    {
      var pathStack = new Stack<bool>(item.Path.Reverse());
      var currNode = _root;
      while (pathStack.Count > 0)
      {
        var nextTossIsHeads = pathStack.Pop();
        currNode = nextTossIsHeads ? currNode.Heads : currNode.Tails;
        if (currNode == null)
        {
          return false;
        }
      }

      return currNode.Equals(item);
    }
    public void CopyTo(INode<T>[] array, int arrayIndex)
    {
      if (array == null)
        throw new ArgumentNullException("array");

      int i = 0;
      foreach(var node in this)
      {
        if(i >= arrayIndex)  array[i++] = node;
      }

      if (array.Length  > i)
        throw new ArgumentException("Not enough elements after arrayIndex in the destination array.");
      //array = array.Skip(arrayIndex - 1).ToArray();
    }
    bool ICollection<INode<T>>.Remove(INode<T> item)
    {
      return this.Remove(item);
    }
    public bool Remove(INode<T> node)
    {
      int nodesInSubtree = node.CountSubsequentNodes(node);
      bool doesContain = Contains(node);

      if (node == _root)
      {
        _root = null;
        Count = 0;
        Time = -1;
        return doesContain;
      }

      if (node.Path.Last())
        node.Previous.Heads = null;
      else
        node.Previous.Tails = null;

      node.Previous = null;

      Count -= nodesInSubtree;
      RecountTime();
      return doesContain;
    }
    public void Clear()
    {
      _root = null;
      Time = 0;
      Count = 0;
    }
    #endregion
    #region ICloneable
    public object Clone() => new BinaryTree<T>((INode<T>)_root.Clone()) { Count = this.Count, Time = this.Time };
    #endregion
    #region IBinaryTree
    public INode<T> GetAt(bool[] path)
    {
      var pathStack = new Stack<bool>(path.Reverse());

      if (pathStack.Count == 0) { return _root; }

      var currNode = _root;
      while (pathStack.Count > 0)
      {
        var nextTossIsHeads = pathStack.Pop();
        currNode = nextTossIsHeads ? currNode.Heads : currNode.Tails;
        if (currNode == null)
        {
          throw new ArgumentException("Could not find a node in this path: " + path, "path");
        }
      }

      return currNode;
    }
    private int RecountTime()
    {
      Time = this.Max(x => x.Path.Length);
      return Time;
    }
    #endregion
  }
}
