using DeltaDerivatives.Maths;
using Xunit;

namespace DeltaTests.Derivatives.AnalyticalOptionPricing
{
    public class EuropeanOptionTests
  {
    [Theory]
    [InlineData( 4, 5, .25, 3, .2, 1.6675924577040089)]
    public void BSMEuropeanCallPrice(double spot, double strike, double rate, double maturity, double volatility, double expPrice)
    {
      Assert.Equal( expPrice, AnalyticalBlackScholes.Calculate( spot, strike, rate, maturity, volatility ) );
    }
  }
}
