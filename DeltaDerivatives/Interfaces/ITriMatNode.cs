namespace DeltaDerivatives.Objects
{
    public interface ITriMatNode<StateType> : ICloneable
    {
        StateType Data { get; set; }
        ITriMatNode<StateType>? ParentHeads { get; set; }
        ITriMatNode<StateType>? ParentTails { get; set; }
        ITriMatNode<StateType>? Heads { get; set; }
        ITriMatNode<StateType>? Tails { get; set; }
        int TimeStep { get; init; }
        int DownMoves { get; init; }
    }
}
