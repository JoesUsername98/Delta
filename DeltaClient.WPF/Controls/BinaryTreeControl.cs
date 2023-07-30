﻿using System;
using System.Buffers.Binary;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace DeltaClient.WPF.Controls
{
    public class BinaryTreeControl : Panel
    {
        public double xElementSeparation { get; set; } = 50;
        public double yElementSeparation { get; set; } = 50;

        protected override Size MeasureOverride(Size availableSize)
        {
            //TODO use my binary tree depth function when imported
            uint totalDepth = Convert.ToUInt16( Math.Ceiling( Math.Log2( Children.Count ) ) );

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
            //TODO use my binary tree depth function when imported
            int totalDepth = Convert.ToInt16( Math.Ceiling( Math.Log2( Children.Count) ) );

            int childI = 1;
            foreach (FrameworkElement child in Children)
            {
                string binary = Convert.ToString(childI, 2);
                int childDepth = binary.Length - binary.IndexOf('1'); 
                int downness = binary.Count(b => b == '1');
                double newPosX = (childDepth-1) * xElementSeparation;
                double newPosY = (this.DesiredSize.Height + child.DesiredSize.Height) / 2 + ((childDepth - 2*downness) * yElementSeparation);

                Point newPos = new Point(newPosX, newPosY);
                   
                child.Arrange(new Rect(newPos, child.DesiredSize));

                childI++;
            }

            return finalSize;
        }
    }
}
