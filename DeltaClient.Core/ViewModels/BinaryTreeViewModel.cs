using DeltaClient.Core.Models;
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
        public BinaryTreeViewModel()
        {
            this.Initialize();
            UpdateTree();
        }

        private void UpdateTree()
        {
            MyBinaryTree = new ObservableCollection<INode<State>>(BinaryTreeFactory.CreateTree(_timePeriods));
        }

        private int _timePeriods = 3;
        public int TimePeriods 
        {
            get
            {
                return _timePeriods;
            }
            set
            {
                _timePeriods = value;
                UpdateTree();
            }
        }

        //the wrapper property
        public ObservableCollection<INode<State>> _myBinaryTree;
        public ObservableCollection<INode<State>> MyBinaryTree
        {
            get
            {
                return _myBinaryTree;
            }
            set
            {
                _myBinaryTree = value;
                RaisePropertyChanged(() => MyBinaryTree);
            }
        }
    }
}

