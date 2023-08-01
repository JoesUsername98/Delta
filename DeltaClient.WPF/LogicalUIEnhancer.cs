using DeltaClient.WPF.Controls;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects;
using DeltaDerivatives.Objects.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DeltaClient.WPF
{
    public static class LogicalUIEnhancer
    {
        public static BinaryTree<UINode<State>, State> CreateTreeObservable(BinaryTree<INode<State>, State> logicalTree)
        { 
            var uiBinaryTree = new BinaryTree<UINode<State>, State>(new UINode<State>(logicalTree.GetAt(new bool[] { }).Data, new bool[] { }));
            for (int currTime = 1; currTime <= logicalTree.Time; currTime++)
            {
                var inputParams = Combinations.GenerateParams(new bool[] { true, false }, currTime);
                foreach (IEnumerable<bool> path in Combinations.Parameters(inputParams))
                {
                    var logicalNode = logicalTree.GetAt(path.ToArray());
                    var uiNode = new UINode<State>(logicalNode.Data, logicalNode.Path);
                    uiBinaryTree.Add(uiNode);
                }
            }

            return uiBinaryTree;
        }
    }
}
