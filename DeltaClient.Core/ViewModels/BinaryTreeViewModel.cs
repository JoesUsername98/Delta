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
        public BinaryTree<Node<State>, State> _tree;
        public ObservableCollection<DependableNode<State>> _observableTree;
        public BinaryTreeViewModel()
        {
            _tree = BinaryTreeFactory.CreateTree(3);//TODO Make dynamic size

            
            //var temp = _tree.Select(a => new DependableNode<State>(a)); //remove investigating
            //_observableTree = new ObservableCollection<DependableNode<State>>(temp);
        }
    }
    //abstract away to factory

}
