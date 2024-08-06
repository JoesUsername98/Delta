using System;
using System.Linq;
using System.Windows.Data;

namespace DeltaClient.WPF.Converters
{
    public class PathToStringConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return String.Join("",values.Select(v => (bool)v ? "h" : "t")) ;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}
