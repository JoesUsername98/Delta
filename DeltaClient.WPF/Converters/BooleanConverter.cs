using System;
using System.Globalization;
using System.Windows.Data;

namespace DeltaClient.WPF.Converters
{
    public class BooleanConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is null)
                return false;

            if (value is bool booleanValue)
            {
                return booleanValue;
            }
            return false;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is null)
                return false;

            if (value is bool booleanValue)
            {
                return booleanValue;
            }
            return false;
        }
    }
}
