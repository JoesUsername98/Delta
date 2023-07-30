using System.Collections;

namespace DeltaDerivatives.Objects.Iterators
{
    public class NodeReverseOrderIterator<T> : IEnumerator<Node<T>>, IEnumerator, IDisposable  where T : IEquatable<T>
  {
    /// <summary>
    /// 1  current is leaf node.
    /// 0  current is yet to be initialised.
    /// -1 current is branch/root node.
    /// </summary>
    private NodeIteratorState state;

    private Node<T> _current;

    public Node<T> _thisNode;

    private Stack<Node<T>> nodeStack;

    private Node<T> _nextNode;

    Node<T> IEnumerator<Node<T>>.Current {
      get {
        return _current;
      }
    }

    object IEnumerator.Current {
      get {
        return _current;
      }
    }

    public NodeReverseOrderIterator(Node<T> thisNode)
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
