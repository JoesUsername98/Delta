using DeltaDerivatives.Objects.Enums;

namespace DeltaDerivatives.Objects.Records
{
    public record BinaryTreeParams(
        int timePeriods,
        double underlyingPrice,
        double upFactor,
        double interestRate,
        OptionPayoffType payoffType,
        double strikePrice,
        OptionExerciseType exerciseType);
}
