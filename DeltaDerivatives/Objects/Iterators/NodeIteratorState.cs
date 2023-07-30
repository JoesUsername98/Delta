namespace DeltaDerivatives.Objects.Iterators
{
  internal enum NodeIteratorState
  {
    CurrentIsLeaf = 1,
    CurrentIsNotInitialized = 0,
    CurrentIsBranch = -1
  }
}
