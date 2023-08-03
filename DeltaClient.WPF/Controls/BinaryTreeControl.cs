using DeltaClient.Core.ViewModels;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public class BinaryTreeControl : Panel
    {
        public readonly BinaryTreeViewModel _treeVM;
        public BinaryTreeControl()
        {
            _treeVM = new BinaryTreeViewModel();
            base.DataContext = _treeVM;
        }
        public double xElementSeparation { get; set; } = 50;
        public double yElementSeparation { get; set; } = 50;
        public bool OverlapNodes { get; set; } = true;

        protected override Size MeasureOverride(Size availableSize)
        {
            return OverlapNodes ? OverlapMeasure(availableSize) : Measure(availableSize);
        }
        protected override Size ArrangeOverride(Size finalSize)
        {
            return OverlapNodes ? OverlapArrange(finalSize) : Arrange(finalSize);
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

            double height = totalDepth * yElementSeparation;
            double width = totalDepth * xElementSeparation;

            return new Size(width, height);
        }

        protected virtual Size Measure(Size availableSize)
        {
            int totalDepth = 0;
            int countMaxDepthNodes = 0;

            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                
                if(nodeChild.Time > totalDepth)
                {
                    countMaxDepthNodes = 0;
                    totalDepth = nodeChild.Time;
                }
                
                if (nodeChild.Time == totalDepth)
                    countMaxDepthNodes++;

                child.Measure(availableSize);
            }

            double height = countMaxDepthNodes * yElementSeparation;
            double width = totalDepth * xElementSeparation;

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
                
                double newPosX = (childDepth) * xElementSeparation;

                double yOffset = (childDepth - 2 * downness) * yElementSeparation;
                double centreLineY = finalSize.Height;
                double newPosY = centreLineY + yOffset ;

                UIChild.Arrange(new Rect(new Point(newPosX, newPosY), UIChild.DesiredSize));
            }
            return finalSize;
        }

        protected virtual Size Arrange(Size finalSize)
        {
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                var UIChild = child as FrameworkElement;

                double yOffset = 0;
                int depthCount = 1;
                foreach (bool hOrT in nodeChild.Path)
                    yOffset += (hOrT ? 1 : -1) * finalSize.Height / Math.Pow(2, depthCount++);

                int childDepth = nodeChild.Time;
                double newPosX = (childDepth) * xElementSeparation;
                double newPosY = finalSize.Height + yOffset;

                UIChild.Arrange(new Rect(new Point(newPosX, newPosY), UIChild.DesiredSize));
            }
            return finalSize;
        }
    }

}
