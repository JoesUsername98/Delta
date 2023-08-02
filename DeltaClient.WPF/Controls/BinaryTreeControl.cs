﻿using DeltaClient.Core.ViewModels;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
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

        protected override Size MeasureOverride(Size availableSize)
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

        protected override Size ArrangeOverride(Size finalSize)
        {
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                var UIChild =   child as FrameworkElement;
                int childDepth = nodeChild.Time;
                int downness = nodeChild.Path.Count(t => t == false);
                double newPosX = (childDepth ) * xElementSeparation;
                //double newPosY = (this.DesiredSize.Height + UIChild.DesiredSize.Height) / 2 + ((childDepth - 2 * downness) * yElementSeparation);
                double newPosY = finalSize.Height + ((childDepth - 2 * downness) * yElementSeparation);

                UIChild.Arrange(new Rect(new Point(newPosX, newPosY), UIChild.DesiredSize));
            }
            return finalSize;
        }

    }

}
