using DeltaDerivatives.Objects.Interfaces;
using System.Collections;

namespace DeltaDerivatives.Objects.Iterators
{
    ///Ripped from sharplab.io when implementing this code (https://stackoverflow.com/questions/27441558/how-to-write-getenumerator-for-a-binary-search-tree)
    ///in Node.GetEnumerator

    /// <summary>
    /// in order traversal
    /// </summary>
    /// <typeparam name="T"></typeparam>
    internal class NodeInOrderIterator<T> : IEnumerator<INode<T>>, IEnumerator, IDisposable where T : IEquatable<T>
  {
    /// <summary>
    /// 1  current is leaf node.
    /// 0  current is yet to be initialised.
    /// -1 current is branch/root node.
    /// </summary>
    private NodeIteratorState state; 

    private INode<T> _current;

    public INode<T> _thisNode;

    private Stack<INode<T>> nodeStack;

    private INode<T> _nextNode;

    INode<T> IEnumerator<INode<T>>.Current 
      {
      get {
        return _current;
      }
    }

    object IEnumerator.Current {
      get {
        return _current;
      }
    }

    public NodeInOrderIterator(INode<T> thisNode)
    {
      _thisNode = thisNode;
      this.state = NodeIteratorState.CurrentIsNotInitialized;
    }

    void IDisposable.Dispose()
    {
    }

    private bool MoveNext()
    {

      if (state != NodeIteratorState.CurrentIsNotInitialized)
      {
        if (state != NodeIteratorState.CurrentIsLeaf)
        {
          return false;
        }
        state = NodeIteratorState.CurrentIsBranch; // current is branch node
        _nextNode = _nextNode.Heads;
      }
      else
      {
        //initiallize iterator
        state = NodeIteratorState.CurrentIsBranch; // current is root note
        nodeStack = new Stack<INode<T>>();
        if (_thisNode is null) return false;
        _nextNode = _thisNode;
      }
      while (nodeStack.Count > 0 || _nextNode != null)
      {
        if (_nextNode != null)
        {
          nodeStack.Push(_nextNode);
          _nextNode = _nextNode.Tails;
          continue;
        }
        _nextNode = nodeStack.Pop();
        _current = _nextNode;
        state = NodeIteratorState.CurrentIsLeaf; // current is leaf node
        return true;
      }

      return false; //enumeration complete when no more items left in stack. 
    }

    bool IEnumerator.MoveNext()
    {
      return this.MoveNext();
    }

    void IEnumerator.Reset()
    {
      throw new NotSupportedException();
    }
  }
}
