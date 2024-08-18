using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;

namespace DeltaDerivatives.Objects
{
    public class TriangularMatrix<N> : 
        INotifyCollectionChanged,
        INotifyPropertyChanged,
        ICloneable,
        IEnumerable<N> 
    {
        public N[][] matrix;
        public TriangularMatrix(int time)
        {
            matrix = new N[time + 1][];
            for (int i = 0; i <= time; ++i)
                matrix[i] = new N[i + 1];
        }
        private TriangularMatrix(N[][] otherMatrix)
        {
            matrix = new N[otherMatrix.Length][];
            for (int time = otherMatrix.Length - 1; time >= 0; time--)
                for (int downMoves = otherMatrix[time].Length - 1; downMoves >= 0; downMoves--)
                    matrix[time][downMoves] = otherMatrix[time][downMoves];
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
        public IEnumerator<N> GetEnumerator()
        {
            for (int time = matrix.Length - 1; time >= 0; time--)
                for (int downMoves = matrix[time].Length - 1; downMoves >= 0; downMoves--)
                    yield return matrix[time][downMoves];
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
        #endregion
        public N this[int i, int j]
        {
            get
            {
                 if (i < j)
                    throw new InvalidOperationException("Cannot get elements below the diagonal in an upper triangular matrix.");
                return matrix[i][j];
            }
            set
            {
                if (i < j)
                    throw new InvalidOperationException("Cannot set elements below the diagonal in an upper triangular matrix.");
                matrix[i][j] = value;
            }
        }
        public N GetAt(bool[] path) => this[path.Length, path.Count(HOrT => !HOrT)];
        #region ICloneable
        public object Clone() => new TriangularMatrix<N>(matrix);
        #endregion
    }
}
