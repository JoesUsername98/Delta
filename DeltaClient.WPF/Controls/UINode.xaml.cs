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
        public bool hasOptionalValue => NodeBT?.Data?.OptimalExerciseTime.HasValue ?? false ;
        #endregion
        #region Public Properties
        public Brush Fill
        {
            get 
            {
                if (!_useTriMat && hasOptionalValue && NodeBT.Data.OptimalExerciseTime.Value == NodeBT.TimeStep)
                    return Brushes.Gold;
                else if (!_useTriMat && hasOptionalValue && NodeBT.Data.OptimalExerciseTime.Value < NodeBT.TimeStep)
                    return Brushes.Black;
                else
                    return Brushes.Orange;
            }
        }
        #endregion
    }
}
