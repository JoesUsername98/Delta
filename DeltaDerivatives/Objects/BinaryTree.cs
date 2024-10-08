﻿using DeltaDerivatives.Objects.Interfaces;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Runtime.Serialization;

namespace DeltaDerivatives.Objects
{
    public class BinaryTree<N, T> : IBinaryTree<N, T> where T : IEquatable<T> where N : INode<T>
    {
        private N _root;
        private int _count;
        private int _timeSteps;
        private double _timeStep;
        private double? _constantUpFactor;
        private double? _constantDownFactor;
        private double? _constantInterestRate;

        public int Count 
        {
          get => _count;
          private set
            {
                _count = value;
                NotifyPropertyChanged(nameof(Count));
            }
        }
        public int TimeSteps 
        {
          get => _timeSteps;
          private set
            {
                _timeSteps = value;
                NotifyPropertyChanged(nameof(TimeSteps));
            } 
        }
        public bool IsReadOnly => false;
        public double TimeStep
        {
            get => _timeStep;
            set
            {
                _timeStep = value;
                NotifyPropertyChanged(nameof(TimeStep));
            }
        }
        public double? ConstantUpFactor 
        {
            get => _constantUpFactor;
            set
            {
                _constantUpFactor = value;
                NotifyPropertyChanged(nameof(ConstantUpFactor));
            }
        }
        public double? ConstantDownFactor
        {
            get => _constantDownFactor;
            set
            {
                _constantDownFactor = value;
                NotifyPropertyChanged(nameof(ConstantDownFactor));
            }
        }
        public double? ConstantInterestRate
        {
            get => _constantInterestRate;
            set
            {
                _constantInterestRate = value;
                NotifyPropertyChanged(nameof(ConstantInterestRate));
            }
        }

        public BinaryTree()
        {
            _root = default(N);
            Count = 0;
            NotifyCollectionReset();
        }
        public BinaryTree(N root)
        {
            _root = root;
            Count = this.Select(x => x).Count();
            NotifyCollectionReset();
        }
        #region IEnumerable
        public IEnumerator<N> GetEnumerator()
        {
            var enumerator = _root.GetForesightEnumerator();
            while (enumerator.MoveNext())
                yield return (N)enumerator.Current;
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
                TimeSteps = 0;
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
            NotifyCollectionAdd(new List<N> { newItem } );
            TimeSteps = newItem.Path.Length > TimeSteps ? newItem.Path.Length : TimeSteps;
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
            foreach (var node in this)
                if (i >= arrayIndex)
                    array[i++] = node;

            if (array.Length > i)
                throw new ArgumentException("Not enough elements after arrayIndex in the destination array.");
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
                TimeSteps = -1;
                NotifyCollectionReset();
                return doesContain;
            }

            if (node.Path.Last())
                node.Previous.Heads = null;
            else
                node.Previous.Tails = null;

            node.Previous = null;

            Count -= nodesInSubtree;
            RecountTimeSteps();
            NotifyCollectionRemove(new List<N> { node });
            return doesContain;
        }
        public void Clear()
        {
            _root = default(N);
            TimeSteps = 0;
            Count = 0;
            NotifyCollectionReset();
        }
        #endregion
        #region ICloneable
        public object Clone() => new BinaryTree<N, T>((N)_root.Clone()) { Count = this.Count, TimeSteps = this.TimeSteps };
        #endregion
        #region INotifyCollectionChanged
        public event NotifyCollectionChangedEventHandler? CollectionChanged;
        private void NotifyCollectionReset()
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }
        private void NotifyCollectionAdd(IList? changedItems)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, changedItems));
        }
        private void NotifyCollectionRemove(IList? changedItems)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, changedItems));
        }
        #endregion
        #region INotifyPropertyChanged
        public event PropertyChangedEventHandler? PropertyChanged;
        private void NotifyPropertyChanged(string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
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
        private int RecountTimeSteps()
        {
            TimeSteps = this.Max(x => x.Path.Length);
            return TimeSteps;
        }
        #endregion
    }
}
