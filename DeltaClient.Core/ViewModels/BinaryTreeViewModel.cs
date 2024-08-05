using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Interfaces;
using DeltaDerivatives.Visitors;
using MvvmCross.ViewModels;
using System.Collections.ObjectModel;

namespace DeltaClient.Core.ViewModels
{
    public class BinaryTreeViewModel : MvxViewModel
    { 
        public BinaryTreeViewModel()
        {
            this.Initialize();
            UpdateTree();
        }

        private void UpdateTree()
        {
            LogicalTree = BinaryTreeFactory.CreateTree(_timePeriods);
            new UnderlyingValueBinaryTreeEnhancer(_underlyingPrice, _upFactor).Enhance(LogicalTree);
            new ConstantInterestRateBinaryTreeEnhancer(_interestRate).Enhance(LogicalTree);
            new PayoffBinaryTreeEnhancer(_payoffType, _strikePrice).Enhance(LogicalTree);
            new RiskNuetralProbabilityEnhancer().Enhance(LogicalTree);
            new ExpectedBinaryTreeEnhancer("PayOff").Enhance(LogicalTree);
            new OptionPriceBinaryTreeEnhancer(_exerciseType).Enhance(LogicalTree);
            new DeltaHedgingBinaryTreeEnhancer().Enhance(LogicalTree);
            DisplayTree = new ObservableCollection<INode<State>>(LogicalTree);
            RaisePropertyChanged(() => OptionValue);
        }

        #region Private Members
        private ObservableCollection<INode<State>> _displayTree;
        private BinaryTree<Node<State>,State> _logicalTree;
        private double _underlyingPrice = 100D;
        private double _strikePrice = 105D;
        private double _upFactor = 2D;
        private double _interestRate = 0.05D;
        private OptionExerciseType _exerciseType = OptionExerciseType.European;
        private OptionPayoffType _payoffType = OptionPayoffType.Call;
        private int _timePeriods = 3;
        #endregion
        #region Public Properties
        public double UnderlyingPrice 
        { 
            get { return _underlyingPrice; } 
            set
            {
                _underlyingPrice = value;
                if (RecalcDynamically) UpdateTree();
            }
        } 
        public double StrikePrice
        {
            get { return _strikePrice; }
            set
            {
                _strikePrice = value;
                if (RecalcDynamically) UpdateTree();
            }
        }

        public double UpFactor 
        {
            get { return _upFactor; }
            set 
            {
                _upFactor = value;
                RaisePropertyChanged(() => DownFactor);
                if (RecalcDynamically) UpdateTree();
            }
        }
        public double DownFactor => 1D/_upFactor; 
        public double InterestRate
        {
            get { return _interestRate; }
            set
            {
                _interestRate = value;
                if (RecalcDynamically) UpdateTree();
            }
        }
        public OptionExerciseType ExerciseType
        {
            get { return _exerciseType; }
            set
            {
                _exerciseType = value;
                if (RecalcDynamically) UpdateTree();
            }
        }
        public OptionPayoffType PayoffType
        {
            get { return _payoffType; }
            set
            {
                _payoffType = value;
                if (RecalcDynamically) UpdateTree();
            }
        }
        public int TimePeriods
        {
            get
            {
                return _timePeriods;
            }
            set
            {
                _timePeriods = value;
                if (RecalcDynamically) UpdateTree();
            }
        }

        bool RecalcDynamically { get; set; } = true;
        public ObservableCollection<INode<State>> DisplayTree
        {
            get
            {
                return _displayTree;
            }
            set
            {
                _displayTree = value;
                RaisePropertyChanged(() => DisplayTree);
            }
        }
        public BinaryTree<Node<State>, State> LogicalTree
        {
            get
            {
                return _logicalTree;
            }
            set
            {
                _logicalTree = value;
                RaisePropertyChanged(() => DisplayTree);
            }
        }
        public double? OptionValue => LogicalTree.GetAt(new bool[] { }).Data.OptionValue; 
        public IEnumerable<OptionExerciseType> ExerciseTypes =>
    Enum.GetValues(typeof(OptionExerciseType)).Cast<OptionExerciseType>();
        public IEnumerable<OptionPayoffType> PayoffTypes =>
            Enum.GetValues(typeof(OptionPayoffType)).Cast<OptionPayoffType>();
        #endregion

    }
}

