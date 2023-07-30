using DeltaClient.WPF.Adapters;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using DeltaDerivatives.Objects.Iterators;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace DeltaClient.WPF.Adapters
{
    //todo make this cleaner
    public partial class MyAdapters : UserControl
    {
        public MyAdapters()
        {
            InitializeComponent();
        }
    }

    //todo make this cleaner
    public static class DependableBinaryTreeFactory
    {
        public static BinaryTree<DependableNode<State>, State> CreateTreeObservable(int time)
        {
            if (time < 0) throw new ArgumentException("time cannot be less than 0", "time");

            var bt = new BinaryTree<DependableNode<State>, State>(new DependableNode<State>(new Node<State>(new State(), new bool[] { })));
            for (int currTime = 1; currTime <= time; currTime++)
            {
                var inputParams = Combinations.GenerateParams(new bool[] { true, false }, currTime);
                foreach (IEnumerable<bool> path in Combinations.Parameters(inputParams))
                    bt.Add(new DependableNode<State>(new Node<State>(new State(), path.ToArray())));
            }
            return bt;
        }
    }

    public class DependableNode<T> : DependencyObject, INode<T> where T : IEquatable<T>
    {
        public INode<T> _node;
        public DependableNode(INode<T> node)
        {
            _node = node;
        }
        public T Data => _node.Data;
        public INode<T> Heads { get => _node.Heads; set => _node.Heads = value; }
        public bool[] Path => _node.Path;
        public INode<T> Previous { get => _node.Previous; set => _node.Previous = value; }
        public INode<T> Tails { get => _node.Tails; set => _node.Tails = value; }
        public int Time => _node.Time;
        public void AddNext(INode<T> nextItem, bool isHeads) => _node.AddNext(nextItem, isHeads);
        public object Clone() => new DependableNode<T>(_node);
        public int CountSubsequentNodes(INode<T> node) => _node.CountSubsequentNodes(node);
        public int CountTime(INode<T> node) => _node.CountTime(node);
        public bool Equals(DependableNode<T>? other) => _node.Equals( other._node );
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
            return new NodeInOrderIterator<DependableNode<T>, T>(this);
        }
        public INode<T> GetNext(bool isHeads) => _node.GetNext(isHeads);
        IEnumerator IEnumerable.GetEnumerator() => _node.GetEnumerator();
    }
}

