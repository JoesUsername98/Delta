using System.Windows.Media;
using System.Windows.Controls;
using System.Drawing;
using System.Windows;
using System.Windows.Shapes;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace DeltaClient.WPF.Controls
{
    public partial class UINode : UserControl, INotifyPropertyChanged
    {
        private Line parentLine;
        public UINode() 
        {
            InitializeComponent();
            this.DataContext = this;

            //Used for debugging purposes
            NodeCount++;
            NodeID = NodeCount;
        }

        public event PropertyChangedEventHandler? PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }

        //Used for debugging purposes
        public int NodeID = 0;
        public static int NodeCount = 0; 

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
        //Used for debugging pupeoses at the moment 
        public string Text
        {
            get { return $"Centre: {Centre},{Centre} \n Parent {ParentCoordinate.X},{ParentCoordinate.Y}"; }
            set { _text = value; }
        }

        //Where the Child-To-Parent line connects from 
        public double Centre
        {
            get { return Height == double.NaN ? 0: Height / 2; }
        }


        //Where the Child-To-Parent line connects to 
        public static readonly DependencyProperty ParentCoordinateProperty =
   DependencyProperty.Register("ParentCoordinate", typeof(System.Windows.Point), typeof(UINode), new
      PropertyMetadata(new PropertyChangedCallback(OnParentCoordinatePropertyChangedCallBack)));

        public System.Windows.Point ParentCoordinate
        {
            get { return (System.Windows.Point)GetValue(ParentCoordinateProperty); }
            set
            {
                SetValue(ParentCoordinateProperty, value);
                OnPropertyChanged("Text");
            }
        }

        private static void OnParentCoordinatePropertyChangedCallBack(DependencyObject d,
   DependencyPropertyChangedEventArgs e)
        {
            UINode UserControl1Control = d as UINode;
            if (UserControl1Control != null)
                UserControl1Control.OnParentCoordinateChanged(e);
        }

        protected virtual void OnParentCoordinateChanged(DependencyPropertyChangedEventArgs e)
        {
            ParentCoordinate = (System.Windows.Point)e.NewValue;
            OnPropertyChanged("ParentCoordinateProperty");
        }
    }
}
