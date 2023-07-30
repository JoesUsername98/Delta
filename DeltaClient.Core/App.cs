using MvvmCross.ViewModels;
using DeltaClient.Core.ViewModels;

namespace DeltaClient.Core
{
    public class App : MvxApplication
    {
        public App()
        {

        }
        public override void Initialize()
        {
            //RegisterAppStart<GuestBookViewModel>();
            RegisterAppStart<BinaryTreeViewModel>();
        }
    }


}
