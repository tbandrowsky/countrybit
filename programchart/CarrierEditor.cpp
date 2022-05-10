#include "pch.h"
#include "CarrierEditor.h"
#if __has_include("CarrierEditor.g.cpp")
#include "CarrierEditor.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    CarrierEditor::CarrierEditor()
    {
        InitializeComponent();
    }

    int32_t CarrierEditor::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void CarrierEditor::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void CarrierEditor::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
