using DeltaDerivatives.Objects.Enums;

namespace DeltaDerivatives.Maths
{
    public class AnalyticalBlackScholes
    {
        private static double getD1(double spot, double strike, double rate, double maturity, double volatility) =>
            (Math.Log(spot / strike) + (rate + volatility * volatility * .5D) * maturity) /
                (volatility * Math.Sqrt(maturity));

        private static double getD2(double spot, double strike, double rate, double maturity, double volatility) =>
            getD1(spot, strike, rate, maturity, volatility) - volatility * Math.Sqrt(maturity);
        private static double CallPrice(double spot, double strike, double rate, double maturity, double volatility) =>
             Stats.CumDensity(getD1(spot, strike, rate, maturity, volatility)) * spot -
                Stats.CumDensity(getD2(spot, strike, rate, maturity, volatility)) * strike * Math.Exp(-rate * maturity);
        private static double PutPrice(double spot, double strike, double rate, double maturity, double volatility) =>
             Stats.CumDensity(-getD2(spot, strike, rate, maturity, volatility)) * strike * Math.Exp(-rate * maturity)
                - Stats.CumDensity(-getD1(spot, strike, rate, maturity, volatility)) * spot;

        public static double Price(OptionPayoffType payoffType, double spot, double strike, double rate, double maturity, double volatility)
        {
            switch (payoffType)
            {
                case OptionPayoffType.Call:
                    return CallPrice(spot, strike, rate, maturity, volatility);
                case OptionPayoffType.Put:
                    return PutPrice(spot, strike, rate, maturity, volatility);
                default:
                    throw new ArgumentException($"Analytical BS does not have an implementation for payoff type {payoffType}");
            }
        }
    }
}