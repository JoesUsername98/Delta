using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;

namespace DeltaDerivatives.Objects
{
    public class TriangularMatrix<NodeStateType, StateType> : 
        INotifyCollectionChanged,
        INotifyPropertyChanged,
        ICloneable,
        IEnumerable<NodeStateType> where NodeStateType : ITriMatNode<StateType>
    {
        public NodeStateType[][] matrix;
        public double dt;
        public TriangularMatrix(int steps, double timeStep = 1D)
        {
            dt = timeStep;
            matrix = new NodeStateType[steps + 1][];
            for (int step = 0; step <= steps; ++step)
                matrix[step] = new NodeStateType[step + 1];
            
        }
        private TriangularMatrix(NodeStateType[][] otherMatrix)
        {
            matrix = new NodeStateType[otherMatrix.Length][];
            for (int step = otherMatrix.Length - 1; step >= 0; step--)
                for (int downMoves = otherMatrix[step].Length - 1; downMoves >= 0; downMoves--)
                    matrix[step][downMoves] = otherMatrix[step][downMoves];
        }
        public int Time => matrix.Length;
        #region INotifyCollectionChanged
        public event NotifyCollectionChangedEventHandler? CollectionChanged;
        private void NotifyCollectionReset()
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }
        private void NotifyCollectionAdd(IList? changedItems)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, changedItems));
        }
        private void NotifyCollectionRemove(IList? changedItems)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, changedItems));
        }
        #endregion
        #region INotifyPropertyChanged
        public event PropertyChangedEventHandler? PropertyChanged;
        private void NotifyPropertyChanged(string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
        #endregion
        #region IEnumerable
        // Traverse the matrix in reverse order (bottom-right to top-left)
        public IEnumerator<NodeStateType> GetEnumerator()
        {
            for (int step = matrix.Length - 1; step >= 0; step--)
                for (int downMoves = matrix[step].Length - 1; downMoves >= 0; downMoves--)
                    yield return matrix[step][downMoves];
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
        #endregion
        public NodeStateType this[int step, int downMoves]
        {
            get
            {
                 if (step < downMoves)
                    throw new InvalidOperationException("Cannot get elements below the diagonal in an upper triangular matrix.");
                return matrix[step][downMoves];
            }
            set
            {
                if (step < downMoves)
                    throw new InvalidOperationException("Cannot set elements below the diagonal in an upper triangular matrix.");
                matrix[step][downMoves] = value;
            }
        }
        public NodeStateType GetAt(bool[] path) => this[path.Length, path.Count(HOrT => !HOrT)];
        #region ICloneable
        public object Clone() => new TriangularMatrix<NodeStateType,StateType>(matrix);
        #endregion
    }
}
