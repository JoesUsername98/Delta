using System.Drawing;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode : UserControl
    {
        public UINode() { }

        private double _diameter = 50;
        public double Diameter
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
