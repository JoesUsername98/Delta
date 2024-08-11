using DeltaClient.WPF.Commands;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Interfaces;
using DeltaDerivatives.Visitors;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Input;

namespace DeltaClient.Core.ViewModels
{
    public class BinaryTreeViewModel : INotifyPropertyChanged
    { 
        public BinaryTreeViewModel()
        {
            UpdateTree();
            ReCalculateCommand = new RelayCommand(ReCalculate, CanRecalculate);
        }
        private void UpdateTree()
        {
            LogicalTree = BinaryTreeFactory.CreateTree(_timePeriods);
            new UnderlyingValueBinaryTreeEnhancer(_underlyingPrice, _upFactor).Enhance(LogicalTree);
            new ConstantInterestRateBinaryTreeEnhancer(_interestRate).Enhance(LogicalTree);
            new PayoffBinaryTreeEnhancer(_payoffType, _strikePrice).Enhance(LogicalTree);
            new RiskNuetralProbabilityEnhancer().Enhance(LogicalTree);
            new ExpectedBinaryTreeEnhancer("PayOff").Enhance(LogicalTree);
            new ExpectedBinaryTreeEnhancer("UnderlyingValue").Enhance(LogicalTree);
            new OptionPriceBinaryTreeEnhancer(_exerciseType).Enhance(LogicalTree);
            new DeltaHedgingBinaryTreeEnhancer().Enhance(LogicalTree);
            if (_exerciseType == OptionExerciseType.American) new StoppingTimeBinaryTreeEnhancer().Enhance(LogicalTree);
            DisplayTree = new ObservableCollection<INode<State>>(LogicalTree);
            OnPropertyChanged(nameof(OptionValue));
        }

        #region Commands
        public ICommand ReCalculateCommand { get; set; }
        private bool CanRecalculate(object obj) => true;
        private void ReCalculate(object obj) { UpdateTree(); }
        #endregion
        #region INotifyPropertyChanged Implementation
        public event PropertyChangedEventHandler? PropertyChanged;
        protected void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)  
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
        #endregion
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
        private bool _recalcDynamically = true;
        #endregion
        #region Public Properties
        public double UnderlyingPrice 
        { 
            get { return _underlyingPrice; } 
            set
            {
                _underlyingPrice = value;
                if (RecalcDynamically) UpdateTree();
                OnPropertyChanged(nameof(UnderlyingPrice));
            }
        } 
        public double StrikePrice
        {
            get { return _strikePrice; }
            set
            {
                _strikePrice = value;
                if (RecalcDynamically) UpdateTree();
                OnPropertyChanged(nameof(StrikePrice));
            }
        }
        public double UpFactor 
        {
            get { return _upFactor; }
            set 
            {
                _upFactor = value;
                if (RecalcDynamically) UpdateTree();
                OnPropertyChanged(nameof(UpFactor));
                OnPropertyChanged(nameof(DownFactor));
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
                OnPropertyChanged(nameof(InterestRate));
            }
        }
        public OptionExerciseType ExerciseType
        {
            get { return _exerciseType; }
            set
            {
                _exerciseType = value;
                if (RecalcDynamically) UpdateTree();
                OnPropertyChanged(nameof(ExerciseType));
            }
        }
        public OptionPayoffType PayoffType
        {
            get { return _payoffType; }
            set
            {
                _payoffType = value;
                if (RecalcDynamically) UpdateTree();
                OnPropertyChanged(nameof(PayoffType));
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
                OnPropertyChanged(nameof(TimePeriods));
            }
        }
        public bool RecalcDynamically 
        {
            get { return _recalcDynamically; } 
            set
            {
                _recalcDynamically = value;
                OnPropertyChanged(nameof(RecalcDynamically));
                OnPropertyChanged(nameof(RecalcManually));
                
                if (_recalcDynamically)
                    UpdateTree();
            }
        } 
        public bool RecalcManually => !RecalcDynamically;
        public ObservableCollection<INode<State>> DisplayTree
        {
            get
            {
                return _displayTree;
            }
            set
            {
                _displayTree = value;
                OnPropertyChanged(nameof(DisplayTree));
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
                OnPropertyChanged(nameof(LogicalTree));
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

