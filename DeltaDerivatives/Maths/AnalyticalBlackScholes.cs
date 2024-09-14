namespace DeltaDerivatives.Maths
{
    public class AnalyticalBlackScholes
    {
        public static double Calculate(double spot, double strike, double rate, double maturity, double volatility)
        {
            double d1 = (Math.Log(spot / strike) + (rate + volatility * volatility * .5D) * maturity) /
                (volatility * Math.Sqrt(maturity));

            double d2 = d1 - volatility * Math.Sqrt(maturity);

            return Stats.CumDensity(d1) * spot - Stats.CumDensity(d2) * strike * Math.Exp( - rate * maturity );
        }
    }


}
