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
            if (ItemContainerGenerator.Items.Count == 0)
                return; 
           
            var tree = new BinaryTree<INode<State>, State>() { ItemContainerGenerator.Items.Cast<INode<State>>().First(n => n.Time == 0) };

            foreach ( var node in tree)
            {
                if (node.Previous is null)
                    continue;

                var nodeVis = (CustomItemContainer)ItemContainerGenerator.ContainerFromItem(node);
                var parentVis = (CustomItemContainer)ItemContainerGenerator.ContainerFromItem(node.Previous);

                var nodeDiam = Math.Min(nodeVis.RenderSize.Width / 2, nodeVis.RenderSize.Height / 2);
                var parentDiam = Math.Min(parentVis.RenderSize.Width / 2, parentVis.RenderSize.Height / 2);

                var nodePoint = nodeVis.TranslatePoint(new Point(nodeDiam, nodeDiam), this);
                var parentPoint = parentVis.TranslatePoint(new Point(parentDiam, parentDiam), this);

                parentPoint.X += parentVis.RenderSize.Width / 2;
                nodePoint.X -= nodeVis.RenderSize.Width / 2;
                
                bool isHeads = node.Path.Last();
                dc.DrawLine(new Pen(isHeads ? Brushes.Navy : Brushes.Crimson, 2), parentPoint, nodePoint);
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
