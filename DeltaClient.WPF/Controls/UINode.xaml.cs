using System.Windows.Media;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode : UserControl
    {
        public UINode() 
        {
            InitializeComponent();
            this.DataContext = this;
        }

        private int _diameter = 50;
        public int Diameter
        {
            get { return _diameter; }
            set { _diameter = value; }
        }

        private Brush _fill = Brushes.Chocolate;
        public Brush Fill
        {
            get { return _fill; }
            set { _fill = value; }
        }

        private string _text = "No binding";
        public string Text
        {
            get { return _text; }
            set { _text = value; }
        }
    }
}
