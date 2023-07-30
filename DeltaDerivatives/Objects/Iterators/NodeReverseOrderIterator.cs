using DeltaDerivatives.Objects.Interfaces;
using System.Collections;

namespace DeltaDerivatives.Objects.Iterators
{
    public class NodeReverseOrderIterator<T> : IEnumerator<INode<T>>, IEnumerator, IDisposable  where T : IEquatable<T>
  {
    /// <summary>
    /// 1  current is leaf node.
    /// 0  current is yet to be initialised.
    /// -1 current is branch/root node.
    /// </summary>
    private NodeIteratorState state;

    private INode<T> _current;

    public INode<T> _thisNode;

    private INode<T> _nextNode;

    INode<T> IEnumerator<INode<T>>.Current {
      get {
        return _current;
      }
    }

    object IEnumerator.Current {
      get {
        return _current;
      }
    }

    public NodeReverseOrderIterator(INode<T> thisNode)
    {
      _thisNode = thisNode;
      this.state = NodeIteratorState.CurrentIsNotInitialized;
    }

    void IDisposable.Dispose()
    {
    }

    public bool MoveNext()
    {
      if (state == NodeIteratorState.CurrentIsNotInitialized)
      {
        if (_thisNode is null) return false;
        _current = _thisNode;
        state = NodeIteratorState.CurrentIsBranch;
      }
      else
      { 
        _current = _nextNode;
      }
      if(_current != null) _nextNode = _current.Previous;
      return _current !=null;

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
