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
        public BinaryTreeControl()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(BinaryTreeControl), new FrameworkPropertyMetadata(typeof(BinaryTreeControl)));
        }

        protected override Size MeasureOverride(Size availableSize)
        {

            if (availableSize.Height == double.PositiveInfinity)
                availableSize.Height = 1500;

            if (availableSize.Width == double.PositiveInfinity)
                availableSize.Width = 1000;

            if (Children.Count == 0)
                return availableSize;

            int totalDepth = 0;

            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                
                if(nodeChild.Time > totalDepth)
                    totalDepth = nodeChild.Time;

                child.Measure(availableSize);
            }

            double xSepToUse = availableSize.Width / Math.Max(totalDepth,1);

            double width = Math.Min( availableSize.Height, totalDepth * xSepToUse);
            double height = availableSize.Height;

            return new Size(width, height);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
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
    }
}
