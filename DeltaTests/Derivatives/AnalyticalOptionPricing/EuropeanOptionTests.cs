using DeltaDerivatives.Maths;
using DeltaDerivatives.Objects.Enums;
using Xunit;

namespace DeltaTests.Derivatives.AnalyticalOptionPricing
{
    public class EuropeanOptionTests
    {
        [Theory]
        [InlineData( 4, 5, .25, 3, .2, 1.6675924577040089)]
        public void BSMEuropeanCallPrice(double spot, double strike, double rate, double maturity, double volatility, double expPrice)
        {
          Assert.Equal( expPrice, AnalyticalBlackScholes.Price( OptionPayoffType.Call, spot, strike, rate, maturity, volatility ) );
        }

        [Theory]
        [InlineData(4, 5, .25, 3, .2, 0.029425221409081936)]
        public void BSMEuropeanPutPrice(double spot, double strike, double rate, double maturity, double volatility, double expPrice)
        {
            Assert.Equal(expPrice, AnalyticalBlackScholes.Price(OptionPayoffType.Put, spot, strike, rate, maturity, volatility));
        }
    }
}
