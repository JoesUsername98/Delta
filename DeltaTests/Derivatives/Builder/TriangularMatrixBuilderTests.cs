using DeltaDerivatives.Builders;
using DeltaDerivatives.Factory;
using DeltaDerivatives.Objects.Enums;
using DeltaDerivatives.Visitors;
using Xunit;

namespace DeltaTests.Derivatives.Builder
{
    /// <summary>
    /// Assert that the TriangularMatrix Values the same as the Binary Tree
    /// </summary>
    public class TriangularMatrixBuilderTests
    {
        [Theory]
        [InlineData(4, 3, 2, 5, 0.25)]
        public void GenerateTriMatEuroCall(int So, int N, double u, double k, double r)
        {
            var tMat = new TriangularMatrixBuilder(N, 1D)
                            .WithUnderlyingValueAndUpFactor(So, u)
                            .WithInterestRate(r)
                            .WithPayoff(OptionPayoffType.Call, k)
                            .WithRiskNuetralProb()
                            .WithPremium(OptionExerciseType.European)
                            .WithDelta()
                            .Build();

            var bTree = BinaryTreeFactory.CreateTree(N, 1D,
                new UnderlyingValueBinaryTreeEnhancer(So, u),
                new ConstantInterestRateBinaryTreeEnhancer(r),
                new PayoffBinaryTreeEnhancer(OptionPayoffType.Call, k),
                new RiskNuetralProbabilityEnhancer(),
                new OptionPriceBinaryTreeEnhancer(OptionExerciseType.European),
                new DeltaHedgingBinaryTreeEnhancer());

            foreach( var bTreeNode in bTree)
            {
                var bTreeState = bTreeNode.Data;
                var tMatState = tMat.GetAt(bTreeNode.Path).Data;
                Assert.True(bTreeState.Equals(tMatState));
            }

        }

        [Theory]
        [InlineData(4, 3, 2, 5, 0.25)]
        public void GenerateTriMatEuroPut(int So, int N, double u, double k, double r)
        {
            var tMat = new TriangularMatrixBuilder(N, 1D)
                            .WithUnderlyingValueAndUpFactor(So, u)
                            .WithInterestRate(r)
                            .WithPayoff(OptionPayoffType.Put, k)
                            .WithRiskNuetralProb()
                            .WithPremium(OptionExerciseType.European)
                            .WithDelta()
                            .Build();

            var bTree = BinaryTreeFactory.CreateTree(N, 1D,
                new UnderlyingValueBinaryTreeEnhancer(So, u),
                new ConstantInterestRateBinaryTreeEnhancer(r),
                new PayoffBinaryTreeEnhancer(OptionPayoffType.Put, k),
                new RiskNuetralProbabilityEnhancer(),
                new OptionPriceBinaryTreeEnhancer(OptionExerciseType.European),
                new DeltaHedgingBinaryTreeEnhancer());

            foreach (var bTreeNode in bTree)
            {
                var bTreeState = bTreeNode.Data;
                var tMatState = tMat.GetAt(bTreeNode.Path).Data;
                Assert.True(bTreeState.Equals(tMatState));
            }

        }

        [Theory]
        [InlineData(4, 3, 2, 5, 0.25)]
        public void GenerateTriMatAmerCall(int So, int N, double u, double k, double r)
        {
            var tMat = new TriangularMatrixBuilder(N, 1D)
                            .WithUnderlyingValueAndUpFactor(So, u)
                            .WithInterestRate(r)
                            .WithPayoff(OptionPayoffType.Call, k)
                            .WithRiskNuetralProb()
                            .WithPremium(OptionExerciseType.American)
                            .WithDelta()
                            .Build();

            var bTree = BinaryTreeFactory.CreateTree(N, 1D,
                new UnderlyingValueBinaryTreeEnhancer(So, u),
                new ConstantInterestRateBinaryTreeEnhancer(r),
                new PayoffBinaryTreeEnhancer(OptionPayoffType.Call, k),
                new RiskNuetralProbabilityEnhancer(),
                new OptionPriceBinaryTreeEnhancer(OptionExerciseType.American),
                new DeltaHedgingBinaryTreeEnhancer());

            foreach (var bTreeNode in bTree)
            {
                var bTreeState = bTreeNode.Data;
                var tMatState = tMat.GetAt(bTreeNode.Path).Data;
                Assert.True(bTreeState.Equals(tMatState));
            }

        }

        [Theory]
        [InlineData(4, 3, 2, 5, 0.25)]
        public void GenerateTriMatAmerPut(int So, int N, double u, double k, double r)
        {
            var tMat = new TriangularMatrixBuilder(N, 1D)
                            .WithUnderlyingValueAndUpFactor(So, u)
                            .WithInterestRate(r)
                            .WithPayoff(OptionPayoffType.Put, k)
                            .WithRiskNuetralProb()
                            .WithPremium(OptionExerciseType.American)
                            .WithDelta()
                            .Build();

            var bTree = BinaryTreeFactory.CreateTree(N, 1D,
                new UnderlyingValueBinaryTreeEnhancer(So, u),
                new ConstantInterestRateBinaryTreeEnhancer(r),
                new PayoffBinaryTreeEnhancer(OptionPayoffType.Put, k),
                new RiskNuetralProbabilityEnhancer(),
                new OptionPriceBinaryTreeEnhancer(OptionExerciseType.American),
                new DeltaHedgingBinaryTreeEnhancer());

            foreach (var bTreeNode in bTree)
            {
                var bTreeState = bTreeNode.Data;
                var tMatState = tMat.GetAt(bTreeNode.Path).Data;
                Assert.True(bTreeState.Equals(tMatState));
            }

        }
    }
}
