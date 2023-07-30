using DeltaClient.WPF.Adapters;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using MvvmCross.Binding.BindingContext;
using MvvmCross.ViewModels;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows;

namespace DeltaClient.Core.ViewModels
{
    //Create UI-friendly wrappers around the raw data objects (i.e. the view-model).
    public class BinaryTreeViewModel : MvxViewModel
    {
        public BinaryTree<DependableNode<State>, State> _tree;
        public ObservableCollection<DependableNode<State>> _observableTree;
        public BinaryTreeViewModel()
        {
            _tree = DependableBinaryTreeFactory.CreateTreeObservable(3);//TODO Make dynamic size
            _observableTree = new ObservableCollection<DependableNode<State>>(_tree);
        }

        public static readonly DependencyProperty ItemSourceProperty = DependencyProperty.Register("ItemSource",
        typeof(ObservableCollection<Node<State>>), typeof(DependableNode<State>), new PropertyMetadata(new PropertyChangedCallback(OnChanged)));

        //the wrapper property
        public ObservableCollection<DependableNode<State>> ItemSource
        {
            get { return _observableTree; }
            set { _observableTree = value; }
        }

        private static void OnChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DependableNode<State> node = (DependableNode<State>)d;
            //node = node.ItemSource;
        }
    }
    //abstract away to factory

}
