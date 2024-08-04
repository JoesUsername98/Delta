using System.Windows.Media;
using System.Windows.Controls;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode : UserControl, INotifyPropertyChanged
    {
        public UINode() 
        {
            InitializeComponent();
            this.DataContext = this;
        }

        public event PropertyChangedEventHandler? PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }

        private Brush _fill = Brushes.Gray;
        public Brush Fill
        {
            get { return _fill; }
            set { _fill = value; }
        }
    }
}
