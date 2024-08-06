using DeltaClient.Core.ViewModels;
using System.Windows;

namespace DeltaClient.WPF.Views
{
    public partial class BinaryTreeView  : Window
    {
        public BinaryTreeView()
        {
            InitializeComponent();
            this.DataContext = new BinaryTreeViewModel();
        }
    }
}
