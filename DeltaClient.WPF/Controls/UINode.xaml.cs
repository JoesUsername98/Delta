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

        #region Events
        private void UINode_Loaded(object sender, RoutedEventArgs e)
        {
            var temp = this.DataContext as INode<State>;
            Node = temp.Clone() as INode<State>;

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
        private Brush _fill = Brushes.Orange;
        public INode<State> Node { get; set; }

        public bool hasOptionalValue => Node.Data.OptimalExerciseTime.HasValue;
        #endregion
        #region Public Properties
        public Brush Fill
        {
            get { return _fill; }
            set
            {
                _fill = value;
                OnPropertyChanged(nameof(Fill));
            }
        }
        #endregion
    }
}
