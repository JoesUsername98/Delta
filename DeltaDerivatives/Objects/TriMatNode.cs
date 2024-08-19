namespace DeltaDerivatives.Objects
{
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
}
