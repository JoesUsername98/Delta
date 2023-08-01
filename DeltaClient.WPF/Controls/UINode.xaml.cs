using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using DeltaDerivatives.Objects.Iterators;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode<T> : UserControl, INode<T> where T : IEquatable<T> 
    {
        #region UI
        private double _diameter = 50;
        public double Diameter
        {
            get { return _diameter; }
            set { _diameter = value; }
        }


        private Color _fill = Color.Wheat;
        public Color Fill
        {
            get { return _fill; }
            set { _fill = value; }
        }

        public string Showtext
        {
            get { return Path.ToString(); }
        }


        #endregion UI
        #region INode
        public INode<T> _node;
        public UINode() { }
        public UINode(INode<T> node)
        {
            _node = node;
        }
        public UINode(T state, bool[] path)
        {
            _node = new Node<T>(state, path);
        }
        public T Data => _node.Data;
        public INode<T> Heads { get => _node.Heads; set => _node.Heads = value; }
        public bool[] Path => _node.Path;
        public INode<T> Previous { get => _node.Previous; set => _node.Previous = value; }
        public INode<T> Tails { get => _node.Tails; set => _node.Tails = value; }
        public int Time => _node.Time;
        public void AddNext(INode<T> nextItem, bool isHeads) => _node.AddNext(nextItem, isHeads);
        public object Clone() => new UINode<T>(_node);
        public int CountSubsequentNodes(INode<T> node) => _node.CountSubsequentNodes(node);
        public int CountTime(INode<T> node) => _node.CountTime(node);
        public bool Equals(UINode<T>? other) => _node.Equals(other._node);
        public bool Equals(INode<T>? other) => _node.Equals(other);
        public IEnumerator<INode<T>> GetEnumerator()
        {
            var enumerator = _node.GetEnumerator();
            while (enumerator.MoveNext())
            {
                yield return (INode<T>)enumerator.Current;
            }
        }
        public IEnumerator<INode<T>> GetForesightEnumerator()
        {
            return new NodeInOrderIterator<UINode<T>, T>(this);
        }
        public INode<T> GetNext(bool isHeads) => _node.GetNext(isHeads);
        IEnumerator IEnumerable.GetEnumerator() => _node.GetEnumerator();
        #endregion INode
    }
}
