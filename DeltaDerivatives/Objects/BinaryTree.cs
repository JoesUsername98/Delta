using DeltaDerivatives.Objects.Interfaces;
using System.Collections;
using System.Runtime.Serialization;

namespace DeltaDerivatives.Objects
{
  public class BinaryTree<N,T> : IBinaryTree<N,T> where T : IEquatable<T> where N : INode<T>
  {
    private N _root;
    public int Count { get; private set; }
    public int Time { get; private set; }
    public bool IsReadOnly => false;
    public double? ConstantUpFactor { get; set; }
    public double? ConstantDownFactor { get; set; }
    public double? ConstantInterestRate { get; set; }
    public BinaryTree()
    {
      _root = default(N);
      Count = 0;
    }
    public BinaryTree(N root)
    {
      _root = root;
      Count = this.Select(x => x).Count();
    }
    #region IEnumerable
    public IEnumerator<N> GetEnumerator()
    {
        var enumerator = _root.GetForesightEnumerator();
        while (enumerator.MoveNext())
        {
            yield return (N)enumerator.Current;
        }
    }
    IEnumerator IEnumerable.GetEnumerator()
    {
      return GetEnumerator();
    }
    #endregion
    #region ICollection
    public void Add(N newItem)
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
        parentNode = nextTossIsHeads ? (N)parentNode.Heads : (N)parentNode.Tails;
        if (parentNode == null)
        {
          throw new ArgumentException("Could not find a parent node in this path: " + parentNodePath, "newItem.Path");
        }
      }

      parentNode.AddNext(newItem, newItem.Path.Last());
      Time = newItem.Path.Length > Time ? newItem.Path.Length : Time;
      Count++;
    }
    public bool Contains(N item)
    {
      var pathStack = new Stack<bool>(item.Path.Reverse());
      var currNode = _root;
      while (pathStack.Count > 0)
      {
        var nextTossIsHeads = pathStack.Pop();
        currNode = nextTossIsHeads ? (N)currNode.Heads : (N)currNode.Tails;
        if (currNode == null)
        {
          return false;
        }
      }

      return currNode.Equals(item);
    }
    public void CopyTo(N[] array, int arrayIndex)
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
    bool ICollection<N>.Remove(N item)
    {
      return this.Remove(item);
    }
    public bool Remove(N node)
    {
      int nodesInSubtree = node.CountSubsequentNodes(node);
      bool doesContain = Contains(node);

      if ((INode<T>)node == (INode<T>)_root)
      {
        _root = default(N);
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
      _root = default(N);
      Time = 0;
      Count = 0;
    }
    #endregion
    #region ICloneable
    public object Clone() => new BinaryTree<N,T>((N)_root.Clone()) { Count = this.Count, Time = this.Time };
    #endregion
    #region IBinaryTree
    public N GetAt(bool[] path)
    {
      var pathStack = new Stack<bool>(path.Reverse());

      if (pathStack.Count == 0) { return _root; }

      var currNode = _root;
      while (pathStack.Count > 0)
      {
        var nextTossIsHeads = pathStack.Pop();
        currNode = nextTossIsHeads ? (N)currNode.Heads : (N)currNode.Tails;
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
