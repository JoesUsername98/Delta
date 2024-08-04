using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace DeltaClient.WPF.Controls
{
    public class BinaryTreeControl : Panel
    {
        public BinaryTreeControl()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(BinaryTreeControl), new FrameworkPropertyMetadata(typeof(BinaryTreeControl)));
        }
        public double xElementSeparation { get; set; } = 50;
        public double yElementSeparation { get; set; } = 50;
        public bool OverlapNodes { get; set; } = true;

        protected override Size MeasureOverride(Size availableSize)
        {
            return OverlapNodes ? OverlapMeasure(availableSize) : NoOverlapMeasure(availableSize);
        }
        protected override Size ArrangeOverride(Size finalSize)
        {
            return OverlapNodes ? OverlapArrange(finalSize) : NoOverlapArrange(finalSize);
        }

        protected virtual Size OverlapMeasure(Size availableSize)
        {
            int totalDepth = 0;

            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                totalDepth = Math.Max(nodeChild.Time, totalDepth);
                child.Measure(availableSize);
            }

            double height = Math.Min(totalDepth * yElementSeparation * 2, 800);
            double width = totalDepth * xElementSeparation;

            return new Size(width, height);
        }

        protected virtual Size NoOverlapMeasure(Size availableSize)
        {
            if (availableSize.Height == double.PositiveInfinity)
                availableSize.Height = 1000;

            if (availableSize.Width == double.PositiveInfinity)
                availableSize.Width = 1000;

            int totalDepth = 0;

            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                
                if(nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;

                child.Measure(availableSize);
            }

            double width = totalDepth * xElementSeparation;
            double height = availableSize.Height;

            return new Size(width, height);
        }

        protected virtual Size OverlapArrange(Size finalSize)
        {
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                var UIChild = child as FrameworkElement;
                int childDepth = nodeChild.Time;
                int downness = nodeChild.Path.Count(t => t == false);

                double yOffset = (childDepth - 2 * downness) * yElementSeparation;
                double centreLineY = (finalSize.Height - UIChild.DesiredSize.Height) / 2;
                double newPosY = centreLineY + yOffset ;

                double newPosX = (childDepth) * xElementSeparation;

                UIChild.Arrange(new Rect(new Point(newPosX, newPosY), UIChild.DesiredSize));
            }
            return finalSize;
        }

        protected virtual Size NoOverlapArrange(Size finalSize)
        {
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                var UIChild = child as FrameworkElement;

                double yOffset = 0;
                int depthCount = 0;
                foreach (bool hOrT in nodeChild.Path)
                    yOffset += (hOrT ? 1 : -1) * (finalSize.Height / 2) / Math.Pow(2, ++depthCount);

                int childDepth = nodeChild.Time;
                double centreLineY = (finalSize.Height - UIChild.DesiredSize.Height) / 2;

                double newPosY = centreLineY + yOffset;
                double newPosX = (childDepth) * xElementSeparation;

                var childHeight = Math.Min(
                    finalSize.Height / Math.Pow(2, nodeChild.Time),
                    UIChild.DesiredSize.Height);

                UIChild.Arrange(
                    new Rect(new Point(newPosX, newPosY),
                    new Size() { Height = childHeight, Width = childHeight })
                    );
            }
            return finalSize;
        }

        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            foreach (ContentPresenter child in InternalChildren)
            {
                var parentNode = child.Content as INode<State>;
                var UIParent = child as FrameworkElement;
                foreach (ContentPresenter otherChild in InternalChildren)
                {
                    var otherNodeChild = otherChild.Content as INode<State>;
                    if (otherNodeChild.Previous != parentNode)
                        continue;
                    var uiOtherChild = otherChild as FrameworkElement;


                    var point1 = UIParent.TranslatePoint(new Point(UIParent.RenderSize.Width / 2, UIParent.RenderSize.Height / 2), this);
                    var point2 = uiOtherChild.TranslatePoint(new Point(uiOtherChild.RenderSize.Width / 2, uiOtherChild.RenderSize.Height / 2), this);

                    dc.DrawLine(new Pen(Brushes.Black, 2), point1, point2);
                }
            }
        }

        private Point GetPositionForChild(UIElement child)
        {
            // Implement your custom logic to determine the position of each child
            // For example, you could arrange items in a circle, grid, etc.
            // This is a simple example placing items in a horizontal line
            int index = InternalChildren.IndexOf(child);
            return new Point(index * 60, 0);
        }
    }

}
