using System;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;

namespace DeltaClient.WPF.Controls
{
    /// <summary>
    /// Dear future self. You read on StackOverflow that VMs on UserControls were bad.
    /// If this goes wrong. Don't hate yourself(myself?). Love Me(you?)
    /// </summary>
    public partial class UINode : UserControl
    {
        public UINode() 
        {
            InitializeComponent();
            Loaded += UINode_Loaded;
        }

        private bool _useTriMat;

        #region Events
        private void UINode_Loaded(object sender, RoutedEventArgs e)
        {
            _useTriMat = this.DataContext is TriMatNode<State>;

            if (_useTriMat )
            {
                var temp = this.DataContext as TriMatNode<State>;
                NodeTriMat = temp.Clone() as TriMatNode<State>;
                NodeBT = null;
            }
            else
            { 
                var temp = this.DataContext as INode<State>;
                NodeBT = temp.Clone() as INode<State>;
                NodeTriMat = null;
            }

            // this works. Consider a VM here. Code is smelly
            this.DataContext = this;
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
        public INode<State>? NodeBT { get; set; }
        public TriMatNode<State>? NodeTriMat { get; set; }
        public object Node => _useTriMat ? NodeTriMat : NodeBT;
        public bool hasOptionalValue 
            => _useTriMat ? 
            NodeTriMat?.Data?.OptimalExerciseTime.HasValue ?? false :
            NodeBT?.Data?.OptimalExerciseTime.HasValue  ?? false ;
        #endregion
        #region Public Properties
        public Brush Fill
        {
            get 
            {
                int? optimalExTime = _useTriMat ? NodeTriMat.Data.OptimalExerciseTime : NodeBT.Data.OptimalExerciseTime;
                int timeStep = _useTriMat ? NodeTriMat.TimeStep : NodeBT.TimeStep;

                if ( hasOptionalValue && optimalExTime.Value == timeStep)
                    return Brushes.Gold;
                else if ( hasOptionalValue && optimalExTime.Value < timeStep)
                    return Brushes.Black;
                else
                    return Brushes.Orange;
            }
        }
        #endregion
    }
}
