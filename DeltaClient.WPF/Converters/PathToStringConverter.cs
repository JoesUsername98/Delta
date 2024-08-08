using System;
using System.Linq;
using System.Windows.Data;

namespace DeltaClient.WPF.Converters
{
    public class PathToStringConverter : IValueConverter
    {
        public object Convert(object values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            var temp = values as bool[];
            if (temp.Length == 0)
                return "root";

            return String.Join("", temp.Select(v => (bool)v ? "H" : "T")) ;
        }

        public object ConvertBack(object value, Type targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value is null)
                return new object[] { };

            var temp = value as string;
            if (temp.Length == 0)
                return new object[] { };

            if (temp.Equals("root"))
                return new object[] { };

            return temp.Select(v => (char)v == 'H' ? true : false).ToList().Cast<object>().ToArray();
        }
    }

}
