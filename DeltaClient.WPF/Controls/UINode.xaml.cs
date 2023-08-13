using System.Windows.Media;
using System.Windows.Controls;
using System.Drawing;
using System.Windows;

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
            get { return $"{parentX} , {parentY}"; }
            set { _text = value; }
        }

        public double centre
        {
            get { return Height == double.NaN ? 0: Height / 2; }
        }

        public double parentX
        {
            get { return -((System.Windows.Point)GetValue(ParentCoordinateProperty)).X; }
        }
        public double parentY
        {
            get { return -((System.Windows.Point)GetValue(ParentCoordinateProperty)).Y; }
        }

        public static readonly DependencyProperty ParentCoordinateProperty =
   DependencyProperty.Register("ParentCoordinate", typeof(System.Windows.Point), typeof(UINode), new
      PropertyMetadata(new System.Windows.Point() { X=30,Y=5}, new PropertyChangedCallback(OnParentCoordinateChanged)));
        public System.Windows.Point ParentCoordinate
        {
            get { return (System.Windows.Point)GetValue(ParentCoordinateProperty); }
            set { SetValue(ParentCoordinateProperty, value); }
        }

        private static void OnParentCoordinateChanged(DependencyObject d,
   DependencyPropertyChangedEventArgs e)
        {
            UINode UserControl1Control = d as UINode;
            UserControl1Control.OnParentCoordinateChanged(e);
        }

        private void OnParentCoordinateChanged(DependencyPropertyChangedEventArgs e)
        {
            ParentCoordinate = (System.Windows.Point)e.NewValue;
        }
    }
}
