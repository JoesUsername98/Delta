using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DeltaClient.WPF.Converters
{
    using System;
    using System.Globalization;
    using System.Windows.Data;
    public class ItemsSourceConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values[0] is bool useTriMat && values[1] != null && values[2] != null)
            {
                return useTriMat ? values[1] : values[2];
            }
            return null;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
