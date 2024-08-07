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
        private void ShowDetailsWindow(object obj)
        {
            String message = $"Path is {String.Join("", Node.Path.Select(v => (bool)v ? "H" : "T"))} \n";
            message += $"Time is {Node.Time} \n";
            message += $"UnderlyingValue is {Node.Data.UnderlyingValue} \n";
            message += $"Payoff is {Node.Data.PayOff} \n";
            message += $"Delta is {Node.Data.DeltaHedging} \n";
            message += $"OptionValue is {Node.Data.OptionValue} \n";
            message += $"Optimal Exercise Time is {Node.Data.OptimalExerciseTime} \n";
            message += $"E[PayOff] is {Node.Data.Expected.PayOff} \n";
            message += $"E[UndelyingValue] is {Node.Data.Expected.UnderlyingValue} \n";

            MessageBox.Show(message, "Node Details", MessageBoxButton.OK, MessageBoxImage.Asterisk);
        }

        #region Events
        private void Node_Click(object sender, RoutedEventArgs e)
        {
            ShowDetailsWindow(new object());
        }
        private void UINode_Loaded(object sender, RoutedEventArgs e)
        {
            var temp = this.DataContext as INode<State>;
            Node = temp.Clone() as INode<State>;
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
        private INode<State> Node { get; set; }
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
