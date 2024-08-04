using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
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

        public decimal? UnderlyingPrice { get; set; } = 100M;

        private decimal _upFactor = 0.5M;
        public decimal UpFactor {
            get { return _upFactor; }
            set 
            {
                _upFactor = value;
                RaisePropertyChanged(() => DownFactor);
            }
        }
        public decimal DownFactor => 1M - _upFactor; 

        public decimal InterestRate { get; set; } = 0.05M;
        public IEnumerable<OptionExerciseType> ExerciseTypes
        {
            get { return Enum.GetValues(typeof(OptionExerciseType)).Cast<OptionExerciseType>(); }
        }
        public IEnumerable<OptionPayoffType> PayoffTypes
        {
            get { return Enum.GetValues(typeof(OptionPayoffType)).Cast<OptionPayoffType>(); }
        }
        public OptionExerciseType ExerciseType { get; set; } = OptionExerciseType.European;
        public OptionPayoffType PayoffType { get; set; } = OptionPayoffType.Call;

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

