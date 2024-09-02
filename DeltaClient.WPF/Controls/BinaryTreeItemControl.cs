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

        public static readonly DependencyProperty UseTriMatProperty =
            DependencyProperty.Register(
            nameof(UseTriMat),
            typeof(bool),
            typeof(BinaryTreeItemControl),
            new PropertyMetadata(true));

        public bool UseTriMat
        {
            get { return (bool)GetValue(UseTriMatProperty); }
            set { SetValue(UseTriMatProperty, value); }
        }

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

        private void ConnectNodesBT(DrawingContext dc)
        {
            if ( ((ContentPresenter)ItemContainerGenerator.ContainerFromIndex(0)).Content is not INode<State>)
                return;

            var tree = new BinaryTree<INode<State>, State>() { ItemContainerGenerator.Items.Cast<INode<State>>().First(n => n.TimeStep == 0) };

            foreach (var node in tree)
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

        private void ConnectNodesTriMat(DrawingContext dc)
        {
            if (((ContentPresenter)ItemContainerGenerator.ContainerFromIndex(0)).Content is not TriMatNode<State>)
                return;

            for (int i = 0; i < ItemContainerGenerator.Items.Count; i++)
            {
                var nodeVis = (ContentPresenter)ItemContainerGenerator.ContainerFromIndex(i);
                var node = nodeVis.Content as TriMatNode<State>;
                var nodeDiam = Math.Min(nodeVis.RenderSize.Width / 2, nodeVis.RenderSize.Height / 2);
                var nodePoint = nodeVis.TranslatePoint(new Point(nodeDiam, nodeDiam), this);
                nodePoint.X -= nodeVis.RenderSize.Width / 2;

                if (node.ParentHeads is not null)
                {
                    var parentHeadsVis = (ContentPresenter)ItemContainerGenerator.ContainerFromItem((TriMatNode<State>)node.ParentHeads);

                    var parentHeadDiam = Math.Min(parentHeadsVis.RenderSize.Width / 2, parentHeadsVis.RenderSize.Height / 2);
                    var parentHeadPoint = parentHeadsVis.TranslatePoint(new Point(parentHeadDiam, parentHeadDiam), this);
                    parentHeadPoint.X += parentHeadsVis.RenderSize.Width / 2;

                    dc.DrawLine(GetColourForLine(node , true), parentHeadPoint, nodePoint);
                }

                if (node.ParentTails is not null)
                {
                    var parentTailsVis = (ContentPresenter)ItemContainerGenerator.ContainerFromItem((TriMatNode<State>)node.ParentTails);

                    var parentTailsDiam = Math.Min(parentTailsVis.RenderSize.Width / 2, parentTailsVis.RenderSize.Height / 2);
                    var parentTailsPoint = parentTailsVis.TranslatePoint(new Point(parentTailsDiam, parentTailsDiam), this);
                    parentTailsPoint.X += parentTailsVis.RenderSize.Width / 2;

                    dc.DrawLine(GetColourForLine(node , false), parentTailsPoint, nodePoint);
                }
            }
        }

        private void ConnectNodes(DrawingContext dc)
        {
            if (ItemContainerGenerator.Items.Count == 0)
                return;

            if( UseTriMat )
                ConnectNodesTriMat(dc);
            else
                ConnectNodesBT(dc);
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

            if (node.TimeStep > node.Data.OptimalExerciseTime.Value) //exercised
                return _exercisedPen;

            return node.Path.Last() ? _upPen : _downPen;
        }

        private Pen GetColourForLine(TriMatNode<State> node, bool isHeads)
        {
            if (!node.Data.OptimalExerciseTime.HasValue) //European
                return isHeads ? _upPen : _downPen;

            var parent = isHeads ? node.ParentHeads : node.ParentTails;
            bool parentExercised = parent.Data.OptimalExerciseTime <= parent.TimeStep;
            if (parentExercised || node.TimeStep > node.Data.OptimalExerciseTime.Value) //exercised
                return _exercisedPen;

            return isHeads ? _upPen : _downPen;
        }
    }
}
