using MvvmCross.ViewModels;
using DeltaClient.Core.ViewModels;

namespace DeltaClient.Core
{
    public class App : MvxApplication
    {
        public override void Initialize()
        {
            RegisterAppStart<BinaryTreeViewModel>();
        }
    }
}
