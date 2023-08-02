using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using MvvmCross.ViewModels;
using System.Collections.ObjectModel;

namespace DeltaClient.Core.ViewModels
{
    //Create UI-friendly wrappers around the raw data objects(i.e.the view-model).
    public class BinaryTreeViewModel : MvxViewModel
    {
        public BinaryTree<Node<State>, State> _tree;

        public BinaryTreeViewModel()
        {
            this.Initialize();
        }

        public override void Prepare()
        {
            _tree = BinaryTreeFactory.CreateTree(3);//TODO Make dynamic size
            _MyBinaryTree = new ObservableCollection<INode<State>>(_tree);
            base.Prepare();
        }

        //the wrapper property
        public ObservableCollection<INode<State>> _MyBinaryTree;
        public ObservableCollection<INode<State>> MyBinaryTree
        {
            get
            {
                return _MyBinaryTree;
            }
            set
            {
                _MyBinaryTree = value;
                RaisePropertyChanged(() => MyBinaryTree);
            }
        }
    }
}

