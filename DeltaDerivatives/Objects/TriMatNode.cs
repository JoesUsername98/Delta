namespace DeltaDerivatives.Objects
{
    public class TriMatNode<StateType> : ITriMatNode<StateType>
    {
        public TriMatNode(int timeStep, int downMoves, StateType data = default(StateType), 
            ITriMatNode<StateType>? parentHeads = null, ITriMatNode<StateType>? parentTails = null,
            ITriMatNode<StateType>? heads = null, ITriMatNode<StateType>? tails = null)
        {
            TimeStep = timeStep;
            DownMoves = downMoves;
            Data = data;
            ParentHeads = parentHeads;
            ParentTails = parentTails;
            Heads = heads;
            Tails = tails;
        }
        public StateType Data { get; set; }
        public ITriMatNode<StateType>? ParentHeads { get; set; }
        public ITriMatNode<StateType>? ParentTails { get; set; }
        public ITriMatNode<StateType>? Heads { get; set; }
        public ITriMatNode<StateType>? Tails { get; set; }
        public int TimeStep { get; init; }
        public int DownMoves { get; init; }

        public object Clone()
        {
            return new TriMatNode<StateType>(this.TimeStep, this.DownMoves, this.Data,
                (TriMatNode<StateType>?)this.ParentHeads,
                (TriMatNode<StateType>?)this.ParentTails,
                (TriMatNode<StateType>?)this.Heads,
                (TriMatNode<StateType>?)this.Tails);
        }
    }
}
