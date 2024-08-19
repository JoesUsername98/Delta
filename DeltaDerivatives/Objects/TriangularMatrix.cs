using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;

namespace DeltaDerivatives.Objects
{
    public interface ITriMatNode<StateType>
    {
        StateType Data { get; set; }
        ITriMatNode<StateType>? ParentHeads { get; init; }
        ITriMatNode<StateType>? ParentTails { get; init; }
        ITriMatNode<StateType>? Heads { get; init; }
        ITriMatNode<StateType>? Tails { get; init; }
        int Time { get; init; }
        int DownMoves { get; init; }
    }

    public class TriMatNode<StateType> : ITriMatNode<StateType>
    {
        public TriMatNode(int time, int downMoves, StateType data = default(StateType), 
            ITriMatNode<StateType>? parentHeads = null, ITriMatNode<StateType>? parentTails = null,
            ITriMatNode<StateType>? heads = null, ITriMatNode<StateType>? tails = null)
        {
            Time = time;
            DownMoves = downMoves;
            Data = data;
            ParentHeads = parentHeads;
            ParentTails = parentTails;
            Heads = heads;
            Tails = tails;
        }
        public StateType Data { get; set; }
        public ITriMatNode<StateType>? ParentHeads { get; init; }
        public ITriMatNode<StateType>? ParentTails { get; init; }
        public ITriMatNode<StateType>? Heads { get; init; }
        public ITriMatNode<StateType>? Tails { get; init; }
        public int Time { get; init; }
        public int DownMoves { get; init; }
    }

    public class TriangularMatrix<NodeStateType, StateType> : 
        INotifyCollectionChanged,
        INotifyPropertyChanged,
        ICloneable,
        IEnumerable<NodeStateType> where NodeStateType : ITriMatNode<StateType>
    {
        public NodeStateType[][] matrix;
        public TriangularMatrix(int maxTime)
        {
            matrix = new NodeStateType[maxTime + 1][];
            for (int time = 0; time <= maxTime; ++time)
                matrix[time] = new NodeStateType[time + 1];
            
        }
        private TriangularMatrix(NodeStateType[][] otherMatrix)
        {
            matrix = new NodeStateType[otherMatrix.Length][];
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
        public IEnumerator<NodeStateType> GetEnumerator()
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
        public NodeStateType this[int time, int downMoves]
        {
            get
            {
                 if (time < downMoves)
                    throw new InvalidOperationException("Cannot get elements below the diagonal in an upper triangular matrix.");
                return matrix[time][downMoves];
            }
            set
            {
                if (time < downMoves)
                    throw new InvalidOperationException("Cannot set elements below the diagonal in an upper triangular matrix.");
                matrix[time][downMoves] = value;
            }
        }
        public NodeStateType GetAt(bool[] path) => this[path.Length, path.Count(HOrT => !HOrT)];
        #region ICloneable
        public object Clone() => new TriangularMatrix<NodeStateType,StateType>(matrix);
        #endregion
    }
}
