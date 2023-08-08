using System.Windows.Media;
using System.Windows.Controls;
using System.Drawing;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode : UserControl
    {
        public UINode() 
        {
            InitializeComponent();
            this.DataContext = this;
        }

        private int _diameter = 30;
        public int Diameter
        {
            get { return _diameter; }
            set
            {
                _diameter = value;
                Height = _diameter;
                Width = _diameter;
            }
        }

        private Brush _fill = Brushes.Gray;
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
