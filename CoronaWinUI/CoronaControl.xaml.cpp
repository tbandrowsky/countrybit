#include "corona.h"
#include "CoronaControl.xaml.h"
#if __has_include("CoronaControl.g.cpp")
#include "CoronaControl.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::CoronaWinUI::implementation
{
    CoronaControl::CoronaControl()
    {
        InitializeComponent();
    }

    int32_t CoronaControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void CoronaControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void CoronaControl::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
