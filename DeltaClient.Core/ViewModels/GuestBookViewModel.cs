using DeltaClient.Core.Models;
using MvvmCross.Commands;
using MvvmCross.ViewModels;
using System.Collections.ObjectModel;

namespace DeltaClient.Core.ViewModels
{
    public class GuestBookViewModel : MvxViewModel
    {
        public GuestBookViewModel()
        {
            AddGuestCommand = new MvxCommand(AddGuest);
            OpenBinaryTreeViewerCommand = new MvxCommand(AddGuest);
        }
        public IMvxCommand AddGuestCommand { get; set; }

        public IMvxCommand OpenBinaryTreeViewerCommand { get; set; }

        public bool CanAddGuest => FirstName?.Length > 0 && LastName?.Length > 0;

        public void AddGuest()
        {
            PersonModel person = new PersonModel { FirstName = FirstName, LastName = LastName };

            FirstName = string.Empty;
            LastName = string.Empty;

            People.Add(person);
        }

        private ObservableCollection<PersonModel> _people = new ObservableCollection<PersonModel>();

        public ObservableCollection<PersonModel> People
        {
            get { return _people; }
            set { SetProperty(ref _people, value); }
        }

        private string _firstName;

        public string FirstName
        {
            get { return _firstName; }
            set 
            { 
                SetProperty(ref _firstName, value);
                RaisePropertyChanged(() => FullName);
                RaisePropertyChanged(() => CanAddGuest);
            }
        }

        private string _lastName;

        public string LastName
        {
            get { return _lastName; }
            set 
            { 
               SetProperty(ref _lastName, value);
               RaisePropertyChanged(() => FullName);
               RaisePropertyChanged(() => CanAddGuest);
            }
        }

        public string FullName => $"{FirstName} {LastName}";

    }
}
