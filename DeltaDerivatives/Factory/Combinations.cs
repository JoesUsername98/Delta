namespace DeltaDerivatives.Factory
{
    public class Combinations
  {
    private static IEnumerable<T> AsSingleton<T>(T source){ yield return source;}
    /// <summary>
    ///https://stackoverflow.com/questions/44894067/combinations-from-different-groups-in-a-specific-order
    /// 
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="ParmLists"></param>
    /// <returns></returns>
    public static IEnumerable<IEnumerable<T>> GenerateParams<T>(IEnumerable<T> uniqueElements, int comboLength)
    {
      var parmLists = new List<List<T>>();
      for (int i = 0; i < comboLength; i++)
      {
        parmLists.Add(uniqueElements.ToList());
      }
      return parmLists;
    }
    public static IEnumerable<IEnumerable<T>> Parameters<T>( IEnumerable<IEnumerable<T>> ParmLists)
    {
      if (ParmLists.Count() == 1)
        foreach (var p in ParmLists.First())
          yield return AsSingleton(p);
      else
      {
        var rest = Parameters(ParmLists.Skip(1));
        foreach (var p in ParmLists.First())
        {
          foreach (var r in rest)
            yield return r.Prepend(p);
        }
      }
    }

    private static long Factorial(int number)
    {
      if (number < 0)
        throw new ArgumentException("Cannot do a factorial of a negative numbers");

      long result = 1;

      for (int i = 1; i <= number; ++i)
        result *= i;

      return result;
    }

    public static long NCR(int n, int r) => Factorial(n) / (Factorial(r) * Factorial(n - r));

    public static long summedNCR(int r) => Enumerable.Range(0, r+1).Sum(i => NCR(r, i));
  }
}
