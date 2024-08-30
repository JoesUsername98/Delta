using DeltaClient.WPF.Commands;
using DeltaDerivatives.Builders;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Objects.Records;
using DeltaDerivatives.Visitors;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Windows.Input;

namespace DeltaClient.Core.ViewModels
{
    public class BinaryTreeViewModel : INotifyPropertyChanged
    {
        // MAKE THIS A SERVICE
        private static readonly Dictionary<string, BinaryTreeParams> Examples = new Dictionary<string, BinaryTreeParams>()
        { 
            { "EuropeanCall", new BinaryTreeParams( 3, 4, 2, 0.25, OptionPayoffType.Call, 5, OptionExerciseType.European ) },
            { "EuropeanPut", new BinaryTreeParams( 3, 4, 2, 0.25, OptionPayoffType.Put, 5, OptionExerciseType.European ) },
            { "AmericanCall", new BinaryTreeParams( 3, 4, 2, 0.25, OptionPayoffType.Call, 5, OptionExerciseType.American ) },
            { "AmericanPut", new BinaryTreeParams( 3, 4, 2, 0.25, OptionPayoffType.Put, 5, OptionExerciseType.American ) },
            { "Exercise4.2", new BinaryTreeParams( 3, 4, 2, 0.25, OptionPayoffType.Put, 5, OptionExerciseType.American ) }
        }
        ;
        public BinaryTreeViewModel()
        {
            UpdateTree();
            ReCalculateCommand = new RelayCommand(ReCalculate, CanRecalculate);
            LoadExampleCommand = new RelayCommand(LoadExample, CanLoadExample);
        }
        private void UpdateTree()
        {
            var timer = new Stopwatch();
            try
            {
                timer.Start();
                if ( UseTriMat )
                {
                    Matrix = new TriangularMatrixBuilder(_timePeriods)
                        .WithUnderlyingValue(_underlyingPrice, _upFactor)
                        .WithInterestRate(_interestRate)
                        .WithPayoff(_payoffType, _strikePrice)
                        .WithRiskNuetralProb()
                        .WithPremium(_exerciseType)
                        .WithDelta()
                        .Build();
                }
                else
                {
                    Tree = BinaryTreeFactory.CreateTree(_timePeriods,
                        new UnderlyingValueBinaryTreeEnhancer(_underlyingPrice, _upFactor),
                        new ConstantInterestRateBinaryTreeEnhancer(_interestRate),
                        new PayoffBinaryTreeEnhancer(_payoffType, _strikePrice),
                        new RiskNuetralProbabilityEnhancer(),
                        new ExpectedBinaryTreeEnhancer("PayOff"),
                        new ExpectedBinaryTreeEnhancer("UnderlyingValue"),
                        new OptionPriceBinaryTreeEnhancer(_exerciseType),
                        new DeltaHedgingBinaryTreeEnhancer(),
                        new StoppingTimeBinaryTreeEnhancer(_exerciseType)
                    );
                }

                OnPropertyChanged(nameof(OptionValue));
                Error = "";
            }
            catch( Exception ex)
            {
                Error = "Error! " + ex.Message;
                Status = "";
            }
            finally
            {
                timer.Stop();
                CalcTime = timer.ElapsedMilliseconds;
            }

        }

        #region Commands
        public ICommand ReCalculateCommand { get; set; }
        private bool CanRecalculate(object obj) => true;
        private void ReCalculate(object obj) { UpdateTree(); }
        public ICommand LoadExampleCommand { get; set; }
        private bool CanLoadExample(object obj) => true;
        private void LoadExample (object obj) 
        {
            if (obj is not string)
                return;

            var nameOfExample = obj as string;
            BinaryTreeParams p = Examples[nameOfExample];
            if (p is null)
                return;
            
            TimePeriods = p.timePeriods;
            UnderlyingPrice = p.underlyingPrice;
            UpFactor = p.upFactor;
            InterestRate = p.interestRate;
            StrikePrice = p.strikePrice;
            ExerciseType = p.exerciseType;
            PayoffType = p.payoffType;
        }
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
        private BinaryTree<Node<State>,State> _tree;
        private TriangularMatrix<TriMatNode<State>, State> _matrix;
        private double _underlyingPrice = 100D;
        private double _strikePrice = 105D;
        private double _upFactor = 2D;
        private double _interestRate = 0.05D;
        private OptionExerciseType _exerciseType = OptionExerciseType.European;
        private OptionPayoffType _payoffType = OptionPayoffType.Call;
        private int _timePeriods = 3;
        private bool _recalcDynamically = true;
        private string _status = "";
        private string _error = "";
        private long _calcTime = 0;
        private bool _overlapNodes = true;
        private bool _useTriMat = true;
        #endregion
        #region Public Properties
        public long CalcTime
        {
            get => _calcTime;
            set
            {
                _calcTime = value;
                OnPropertyChanged(nameof(CalcTime));
                OnPropertyChanged(nameof(Status));
                OnPropertyChanged(nameof(HasStatus));
            }
        }
        public bool HasStatus => _status.Length > 0;
        public string Status
        {
            get
            {
                _status = $"Calc Duration {CalcTime} ms";
                return _status;
            }
            set
            {
                _status = value;
                OnPropertyChanged(nameof(Status));
                OnPropertyChanged(nameof(HasStatus));
            }
        }
        public bool HasError => _error.Length > 0;
        public string Error
        {
            get { return _error; }
            set
            {
                _error = value;
                OnPropertyChanged(nameof(Error));
                OnPropertyChanged(nameof(HasError));
            }
        }
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
        public bool OverlapNodes 
        {
            get { return _overlapNodes; }
            set
            {
                if (_overlapNodes != value)
                {
                    _overlapNodes = value;

                    if (!_overlapNodes && UseTriMat)
                        UseTriMat = false;

                    OnPropertyChanged(nameof(OverlapNodes));
                    if (RecalcDynamically) UpdateTree(); // hack to redraw
                }
            }
        }
        public bool UseTriMat
        {
            get { return _useTriMat; }
            set
            {
                if (_useTriMat != value)
                {
                    _useTriMat = value;

                    if (!OverlapNodes && _useTriMat)
                        OverlapNodes = true;

                    OnPropertyChanged(nameof(UseTriMat));
                    if (RecalcDynamically) UpdateTree(); // hack to redraw
                }
            }
        }
        public BinaryTree<Node<State>, State> Tree
        {
            get
            {
                return _tree;
            }
            set
            {
                _tree = value;
                OnPropertyChanged(nameof(Tree));
                OnPropertyChanged(nameof(NodeCollection));
            }
        }
        public TriangularMatrix<TriMatNode<State>, State> Matrix
        {
            get
            {
                return _matrix;
            }
            set
            {
                _matrix = value;
                OnPropertyChanged(nameof(Matrix));
                OnPropertyChanged(nameof(NodeCollection));
            }
        }
        public IEnumerable<object> NodeCollection
        {
            get
            {
                return (UseTriMat ? Matrix : Tree);
            }
        }

        public double? OptionValue => 
            UseTriMat ? 
                Matrix[0,0].Data.OptionValue : 
                Tree.GetAt(new bool[] { }).Data.OptionValue; 
        public IEnumerable<OptionExerciseType> ExerciseTypes =>
    Enum.GetValues(typeof(OptionExerciseType)).Cast<OptionExerciseType>();
        public IEnumerable<OptionPayoffType> PayoffTypes =>
            Enum.GetValues(typeof(OptionPayoffType)).Cast<OptionPayoffType>();
        #endregion

    }
}

