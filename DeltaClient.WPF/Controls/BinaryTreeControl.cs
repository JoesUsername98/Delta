using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public class BinaryTreeControl : Canvas
    {
        public static readonly DependencyProperty OverlapProperty =
            DependencyProperty.Register(
                nameof(Overlap),                
                typeof(bool),                   
                typeof(BinaryTreeControl),      
                new PropertyMetadata(true));    

        public bool Overlap
        {
            get { return (bool)GetValue(OverlapProperty); }
            set 
            { 
                SetValue(OverlapProperty, value); 
                InvalidateMeasure();
                InvalidateArrange();
            }
        }

        public static readonly DependencyProperty UseTriMatProperty =
    DependencyProperty.Register(
        nameof(UseTriMat),
        typeof(bool),
        typeof(BinaryTreeControl),
        new PropertyMetadata(true));

        public bool UseTriMat
        {
            get { return (bool)GetValue(UseTriMatProperty); }
            set
            {
                SetValue(UseTriMatProperty, value);
                if (value && !Overlap)
                    Overlap = true;
            }
        }

        public BinaryTreeControl()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(BinaryTreeControl), new FrameworkPropertyMetadata(typeof(BinaryTreeControl)));
        }

        private int MeasureChildrenBT (Size availableSize)
        {
            int totalDepth = 0;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;

                if (nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;

                child.Measure(availableSize);
            }
            return totalDepth;
        }

        private int MeasureChildrenTriMat(Size availableSize)
        {
            int totalDepth = 0;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as TriMatNode<State>;

                if (nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;

                child.Measure(availableSize);
            }
            return totalDepth;
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (availableSize.Height == double.PositiveInfinity)
                availableSize.Height = 1500;

            if (availableSize.Width == double.PositiveInfinity)
                availableSize.Width = 1000;
            
            if (Children.Count == 0)
                return availableSize;

            double totalDepth = UseTriMat ? MeasureChildrenTriMat(availableSize) : MeasureChildrenBT(availableSize);

            double xSepToUse = availableSize.Width / Math.Max(totalDepth,1);

            double width = Math.Min( availableSize.Height, totalDepth * xSepToUse);
            double height = availableSize.Height;

            return new Size(width, height);
        }
        
        protected override Size ArrangeOverride(Size finalSize)
        {
            return Overlap ? ArrangeOverlap(finalSize) : ArrangeNoOverlap(finalSize);
        }
        private Size ArrangeNoOverlap(Size finalSize)
        {
            if (UseTriMat)
                throw new InvalidOperationException("Nodes must overlap with a Triangular Matrix Calculation method!");

            if (Children.Count == 0)
                return finalSize;

            int totalDepth = 0;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;

                if (nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;
            }

            double maxNodeSize = 0;
            double minNodeSize = 0;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;

                var UIChild = child as FrameworkElement;

                double height = Math.Min(finalSize.Height / Math.Pow(2, nodeChild.Time),
                                UIChild.DesiredSize.Height);
                double width = Math.Min(finalSize.Width / (nodeChild.Time + 1),
                 UIChild.DesiredSize.Height);

                maxNodeSize = Math.Max(Math.Max(maxNodeSize, height), width);
                minNodeSize = Math.Min(Math.Min(maxNodeSize, height), width);
            }
            double xSepToUse = (finalSize.Width - minNodeSize) / Math.Max(totalDepth, 1);

            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;

                var UIChild = child as FrameworkElement;

                double yOffset = 0;
                int depthCount = 0;
                foreach (bool hOrT in nodeChild.Path)
                        yOffset += (hOrT ? -1 : 1) * (finalSize.Height / 2) / Math.Pow(2, ++depthCount);

                int childDepth = nodeChild.Time;

                double newPosX = childDepth * xSepToUse;
                double centreLineY = (finalSize.Height - minNodeSize) / 2;
                double newPosY = centreLineY + yOffset;

                UIChild.Arrange(
                    new Rect(new Point(newPosX, newPosY),
                    new Size() { Height = minNodeSize, Width = minNodeSize })
                    );
            }
            return finalSize;
        }

        private int GetTotalDepthFromChildTriMat()
        {
            int totalDepth = 0;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as TriMatNode<State>;

                if (nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;
            }
            return totalDepth;
        }

        private int GetTotalDepthFromChildBT()
        {
            int totalDepth = 0;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;

                if (nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;
            }
            return totalDepth;
        }

        private double GetChildMinSizeTriMat(Size finalSize)
        {
            double maxNodeSize = double.MinValue;
            double minNodeSize = double.MaxValue;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as TriMatNode<State>;

                var UIChild = child as FrameworkElement;

                double height = Math.Min(finalSize.Height / (nodeChild.Time + 1),
                                UIChild.DesiredSize.Height);
                double width = Math.Min(finalSize.Width / (nodeChild.Time + 1),
                 UIChild.DesiredSize.Height);

                double smallerDim = Math.Min(width, height);
                double largerDim = Math.Max(width, height);

                maxNodeSize = Math.Max(maxNodeSize, largerDim);
                minNodeSize = Math.Min(minNodeSize, smallerDim);
            }
            return minNodeSize;
        }

        private double GetChildMinSizeBT(Size finalSize)
        {
            double maxNodeSize = double.MinValue;
            double minNodeSize = double.MaxValue;
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;

                var UIChild = child as FrameworkElement;

                double height = Math.Min(finalSize.Height / (nodeChild.Time + 1),
                                UIChild.DesiredSize.Height);
                double width = Math.Min(finalSize.Width / (nodeChild.Time + 1),
                 UIChild.DesiredSize.Height);

                double smallerDim = Math.Min(width, height);
                double largerDim = Math.Max(width, height);

                maxNodeSize = Math.Max(maxNodeSize, largerDim);
                minNodeSize = Math.Min(minNodeSize, smallerDim);
            }
            return minNodeSize;
        }

        private Point GetChildPositionBT(ContentPresenter child, int totalDepth, double xSepToUse, double minNodeSize, Size finalSize)
        {
            var nodeChild = child.Content as INode<State>;

            double yOffset = 0;
            foreach (bool hOrT in nodeChild.Path)
                yOffset += (hOrT ? -1D : 1D) / (1D + totalDepth);

            double newPosX = nodeChild.Time * xSepToUse;
            double centreLineY = (finalSize.Height - minNodeSize) / 2;
            double newPosY = centreLineY + yOffset * (finalSize.Height / 2);

            return new Point(newPosX, newPosY);
        }

        private Point GetChildPositionTriMat(ContentPresenter child, int totalDepth, double xSepToUse, double minNodeSize, Size finalSize)
        {
            var nodeChild = child.Content as TriMatNode<State>;
            double yOffset = (2 * nodeChild.DownMoves - nodeChild.Time) / (1D + totalDepth);

            double newPosX = nodeChild.Time * xSepToUse;
            double centreLineY = (finalSize.Height - minNodeSize) / 2;
            double newPosY = centreLineY + yOffset * (finalSize.Height / 2);

            return new Point(newPosX, newPosY);
        }

        private Size ArrangeOverlap(Size finalSize)
        {
            if (Children.Count == 0)
                return finalSize;

            int totalDepth = UseTriMat ? GetTotalDepthFromChildTriMat() : GetTotalDepthFromChildBT();
            double minNodeSize = UseTriMat ? GetChildMinSizeTriMat(finalSize) : GetChildMinSizeBT(finalSize);
            double xSepToUse = (finalSize.Width - minNodeSize) / Math.Max(totalDepth, 1);

            foreach (ContentPresenter child in Children)
            {
                var UIChild = child as FrameworkElement;

                Point childPos = UseTriMat ?
                    GetChildPositionTriMat(child, totalDepth, xSepToUse, minNodeSize, finalSize) :
                    GetChildPositionBT(child, totalDepth, xSepToUse, minNodeSize, finalSize) ;

                UIChild.Arrange( new Rect(childPos, new Size() { Height = minNodeSize, Width = minNodeSize }) );
            }
            return finalSize;
        }
    }
}
