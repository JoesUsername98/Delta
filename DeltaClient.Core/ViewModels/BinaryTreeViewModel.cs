using DeltaClient.WPF.Adapters;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using MvvmCross.Binding.BindingContext;
using MvvmCross.ViewModels;
using System.Collections.ObjectModel;
using System.ComponentModel;

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
    }
    //abstract away to factory

}
