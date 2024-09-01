using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DeltaDerivatives.Objects
{
  public class ExpectableState : IEquatable<ExpectableState>
  {
    public double UnderlyingValue { get; set; }
    public double PayOff { get; set; }
    private double _interestRate;
    public double InterestRate {
      get { return _interestRate; }
      set 
      {
        if (value == -1) throw new InvalidOperationException("InterestRate cannot be -1");
        _interestRate = value;
      }
    }
    public double DiscountRate => 1D / (1D + InterestRate); 
    public bool Equals(ExpectableState other)
    {
      return PayOff == other.PayOff &&
              InterestRate == other.InterestRate;
    }
  }
}
