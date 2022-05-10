#include "pch.h"
#include "CoverageEditor.h"
#if __has_include("CoverageEditor.g.cpp")
#include "CoverageEditor.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    CoverageEditor::CoverageEditor()
    {
        InitializeComponent();
    }

    int32_t CoverageEditor::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void CoverageEditor::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void CoverageEditor::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
