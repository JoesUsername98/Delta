using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Collections;
using System.Collections.Specialized;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DeltaClient.WPF.Controls
{
    public class BinaryTreeItemControl : ItemsControl
    {
        private INotifyCollectionChanged _itemsSource;

        public BinaryTreeItemControl()
        {
            Loaded += CustomItemsControl_Loaded;
        }

        private void CustomItemsControl_Loaded(object sender, RoutedEventArgs e)
        {
            AttachCollectionChangedHandler(ItemsSource as INotifyCollectionChanged);
        }

        protected override void OnItemsSourceChanged(IEnumerable oldValue, IEnumerable newValue)
        {
            base.OnItemsSourceChanged(oldValue, newValue);

            DetachCollectionChangedHandler(oldValue as INotifyCollectionChanged);
            AttachCollectionChangedHandler(newValue as INotifyCollectionChanged);

            this.InvalidateVisual(); // Redraw lines when ItemsSource changes
        }
        private void AttachCollectionChangedHandler(INotifyCollectionChanged newCollection)
        {
            if (newCollection != null)
            {
                newCollection.CollectionChanged += ItemsSource_CollectionChanged;
                _itemsSource = newCollection;
            }
        }

        private void DetachCollectionChangedHandler(INotifyCollectionChanged oldCollection)
        {
            if (oldCollection != null)
            {
                oldCollection.CollectionChanged -= ItemsSource_CollectionChanged;
                _itemsSource = null;
            }
        }

        private void ItemsSource_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            this.InvalidateVisual();
        }

        public class CustomItemContainer : ContentPresenter
        {
            public Point Position { get; set; }

            public CustomItemContainer()
            {
                this.Loaded += CustomItemContainer_Loaded;
            }

            private void CustomItemContainer_Loaded(object sender, RoutedEventArgs e)
            {
                Position = TranslatePoint(new Point(0, 0), (UIElement)VisualParent);
                UpdateLayout();
            }
        }

        protected override DependencyObject GetContainerForItemOverride()
        {
            return new CustomItemContainer();
        }

        protected override bool IsItemItsOwnContainerOverride(object item)
        {
            return item is CustomItemContainer;
        }

        protected override void OnRender(DrawingContext dc)
        {
            base.OnRender(dc);

            for (int i = 0; i < Items.Count; i++)
            {
                var node_i = (CustomItemContainer)ItemContainerGenerator.ContainerFromIndex(i);
                var node = node_i.Content as INode<State>;
                for (int j = 0; j < Items.Count; j++)
                {
                    var node_j = (CustomItemContainer)ItemContainerGenerator.ContainerFromIndex(j);
                    var otherNode = node_j.Content as INode<State>;
                    if (otherNode.Previous != node)
                        continue;

                    var point1 = node_i.TranslatePoint(new Point(node_i.RenderSize.Width / 2, node_i.RenderSize.Height / 2), this);
                    var point2 = node_j.TranslatePoint(new Point(node_j.RenderSize.Width / 2, node_j.RenderSize.Height / 2), this);

                    dc.DrawLine(new Pen(Brushes.Black, 2), point1, point2);
                }
            }
        }

    }

}
