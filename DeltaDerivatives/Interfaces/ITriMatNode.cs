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
}
