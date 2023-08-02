using System.Drawing;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode : UserControl
    {
        public UINode() 
        {
            InitializeComponent();
        }

        private int _diameter = 50;
        public int Diameter
        {
            get { return _diameter; }
            set { _diameter = value; }
        }

        private Color _fill = Color.Wheat;
        public Color Fill
        {
            get { return _fill; }
            set { _fill = value; }
        }
    }
}
