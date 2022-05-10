#include "pch.h"
#include "DropDownInputControl.h"
#if __has_include("DropDownInputControl.g.cpp")
#include "DropDownInputControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    DropDownInputControl::DropDownInputControl()
    {
        InitializeComponent();
    }

    int32_t DropDownInputControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void DropDownInputControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void DropDownInputControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
