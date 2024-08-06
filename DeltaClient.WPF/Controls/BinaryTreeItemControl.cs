using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DeltaClient.WPF.Controls
{
    public class BinaryTreeItemControl : ItemsControl
    {
        private DrawingVisual _lineVisual;

        public BinaryTreeItemControl()
        {
            LayoutUpdated += BinaryTreeItemControl_LayoutUpdated;
            _lineVisual = new DrawingVisual();
            AddVisualChild(_lineVisual);
        }
        private void BinaryTreeItemControl_LayoutUpdated(object sender, EventArgs e)
        {
            RedrawLines();
        }

        protected override DependencyObject GetContainerForItemOverride()
        {
            return new CustomItemContainer();
        }

        protected override bool IsItemItsOwnContainerOverride(object item)
        {
            return item is CustomItemContainer;
        }

        protected override int VisualChildrenCount => base.VisualChildrenCount + 1;

        protected override Visual GetVisualChild(int index)
        {
            if (index < base.VisualChildrenCount)
                return base.GetVisualChild(index);
            return _lineVisual;
        }

        private void RedrawLines()
        {
            using (DrawingContext dc = _lineVisual.RenderOpen())
            {
                ConnectNodes(dc);
            }
        }

        private void ConnectNodes(DrawingContext dc)
        {
            for (int parentIdx = 0; parentIdx < Items.Count; parentIdx++)
            {
                var parentVisual = (CustomItemContainer)ItemContainerGenerator.ContainerFromIndex(parentIdx);
                var parent = parentVisual.Content as INode<State>;
                var parentDiam = Math.Min(parentVisual.RenderSize.Width / 2, parentVisual.RenderSize.Height / 2);
                for (int childIdx = 0; childIdx < Items.Count; childIdx++)
                {
                    var childVisual = (CustomItemContainer)ItemContainerGenerator.ContainerFromIndex(childIdx);
                    var otherNode = childVisual.Content as INode<State>;
                    if (otherNode.Previous != parent)
                        continue;
                    
                    bool isHeads = otherNode.Path.Last();
                    var childDiam = Math.Min(childVisual.RenderSize.Width / 2, childVisual.RenderSize.Height / 2);
                    var parentPt = parentVisual.TranslatePoint(new Point(parentDiam, parentDiam), this);
                    var childPoint = childVisual.TranslatePoint(new Point(childDiam, childDiam), this);
                    //Move to front/end of nodes
                    parentPt.X += parentVisual.RenderSize.Width / 2;
                    childPoint.X -= childVisual.RenderSize.Width / 2;

                    dc.DrawLine(new Pen(isHeads ? Brushes.Navy : Brushes.Crimson, 2), parentPt, childPoint);
                }
            }
        }

        public class CustomItemContainer : ContentPresenter
        {
            public Point Position { get; set; }

            public CustomItemContainer()
            {
                this.Loaded += CustomItemContainer_Loaded;
                this.SizeChanged += CustomItemContainer_SizeChanged;
            }

            private void CustomItemContainer_Loaded(object sender, RoutedEventArgs e)
            {
                UpdatePosition();
                InvalidateParentVisual();
            }

            private void CustomItemContainer_SizeChanged(object sender, SizeChangedEventArgs e)
            {
                UpdatePosition();
                InvalidateParentVisual();
            }

            private void UpdatePosition()
            {
                Position = TranslatePoint(new Point(0, 0), (UIElement)VisualParent);
            }

            private void InvalidateParentVisual()
            {
                var parent = VisualParent as UIElement;
                parent?.InvalidateVisual();
            }
        }
    }
}
