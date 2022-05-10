#include "pch.h"
#include "DeductibleEditor.h"
#if __has_include("DeductibleEditor.g.cpp")
#include "DeductibleEditor.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    DeductibleEditor::DeductibleEditor()
    {
        InitializeComponent();
    }

    int32_t DeductibleEditor::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void DeductibleEditor::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void DeductibleEditor::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
