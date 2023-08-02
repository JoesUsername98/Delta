using DeltaClient.Core.ViewModels;
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
            int totalDepth = Children.Count == 0 ? 0 : Convert.ToUInt16(Math.Ceiling(Math.Log2(Children.Count)));

            double height = totalDepth * yElementSeparation * 2 ;
            double width = totalDepth * xElementSeparation;

            foreach (FrameworkElement child in Children)
            {
                child.Measure(availableSize);
                height += child.DesiredSize.Height;
                width += child.DesiredSize.Width;
            }

            return new Size(width, height);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                var UIChild = (FrameworkElement)child;
                int childDepth = nodeChild.Time;
                int downness = nodeChild.Path.Count(t => t == false);
                double newPosX = (childDepth ) * xElementSeparation;
                double newPosY = (this.DesiredSize.Height + UIChild.DesiredSize.Height) / 2 + ((childDepth - 2 * downness) * yElementSeparation);

                UIChild.Arrange(new Rect(new Point(newPosX, newPosY), UIChild.DesiredSize));
            }
            return finalSize;
        }

    }

}
