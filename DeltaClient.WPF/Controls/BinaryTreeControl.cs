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
            //TODO use my binary tree depth function when imported
            //int totalDepth = _tree.Time;
            int totalDepth = Children.Count == 0 ? 0 : Convert.ToUInt16(Math.Ceiling(Math.Log2(Children.Count)));

            double height = totalDepth * yElementSeparation * 2 ;
            double width = totalDepth * xElementSeparation;

            //UINode<State> child
            foreach (ContentPresenter child in Children)
            {
                child.Measure(availableSize);
                height += Math.Max(child.DesiredSize.Height, child.MinHeight);
                width += Math.Max(child.DesiredSize.Width, child.MinWidth); //testing
            }

            return new Size(width, height);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            //TODO use my binary tree depth function when imported
            int childI = 1;

            //UINode<State> child
            foreach (ContentPresenter child in Children)
            {
                var nodeChild = child.Content as INode<State>;
                var UIChild = (FrameworkElement)child;
                string binary = Convert.ToString(childI, 2);
                int childDepth = nodeChild.Time;
                int downness = nodeChild.Path.Count(t => t == false);
                double newPosX = (childDepth - 1) * xElementSeparation;
                double newPosY = (this.DesiredSize.Height + UIChild.DesiredSize.Height) / 2 + ((childDepth - 2 * downness) * yElementSeparation);

                UIChild.Arrange(new Rect(new Point(newPosX, newPosY), new Size() { Height = 50, Width = 50 }));
                //UIChild.Arrange(new Rect(new Point(200, 200), new Size() { Height = 50, Width = 50 }));

                childI++;
            }
            return finalSize;
        }

    }

}
