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

                var nodeVis = (UIElement)ItemContainerGenerator.ContainerFromItem(node);
                var parentVis = (UIElement)ItemContainerGenerator.ContainerFromItem(node.Previous);

                var nodeDiam = Math.Min(nodeVis.RenderSize.Width / 2, nodeVis.RenderSize.Height / 2);
                var parentDiam = Math.Min(parentVis.RenderSize.Width / 2, parentVis.RenderSize.Height / 2);

                var nodePoint = nodeVis.TranslatePoint(new Point(nodeDiam, nodeDiam), this);
                var parentPoint = parentVis.TranslatePoint(new Point(parentDiam, parentDiam), this);

                parentPoint.X += parentVis.RenderSize.Width / 2;
                nodePoint.X -= nodeVis.RenderSize.Width / 2;
                
                dc.DrawLine(GetColourForLine(node), parentPoint, nodePoint);
            }
        }
        private static readonly Pen _exercisedPen = new Pen()
        {
            Brush = Brushes.Black,
            Thickness = 1,
            DashStyle = new DashStyle(new double[] { 1,2,3}, 1)
        };
        private static readonly Pen _upPen = new Pen(){ Brush = Brushes.Navy, Thickness = 2 };
        private static readonly Pen _downPen = new Pen() { Brush = Brushes.Crimson, Thickness = 2 };

        private Pen GetColourForLine(INode<State> node)
        {
            if( !node.Data.OptimalExerciseTime.HasValue ) //European
                return node.Path.Last() ? _upPen : _downPen;

            if (node.Time > node.Data.OptimalExerciseTime.Value) //exercised
                return _exercisedPen;

            return node.Path.Last() ? _upPen : _downPen;
        }
    }
}
