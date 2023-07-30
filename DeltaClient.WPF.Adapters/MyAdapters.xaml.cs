using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace DeltaClient.WPF.Adapters
{
    /// <summary>
    /// Interaction logic for MyAdapters.xaml
    /// </summary>
    public partial class MyAdapters : UserControl
    {
        public MyAdapters()
        {
            InitializeComponent();
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
        public IEnumerator<INode<T>> GetEnumerator() => _node.GetEnumerator();
        public IEnumerator<INode<T>> GetForesightEnumerator() => _node.GetForesightEnumerator();
        public INode<T> GetNext(bool isHeads) => _node.GetNext(isHeads);
        IEnumerator IEnumerable.GetEnumerator() => _node.GetEnumerator();
    }
}
