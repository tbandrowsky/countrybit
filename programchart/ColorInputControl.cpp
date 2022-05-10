#include "pch.h"
#include "ColorInputControl.h"
#if __has_include("ColorInputControl.g.cpp")
#include "ColorInputControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    ColorInputControl::ColorInputControl()
    {
        InitializeComponent();
    }

    int32_t ColorInputControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ColorInputControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void ColorInputControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
