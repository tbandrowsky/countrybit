#include "pch.h"
#include "ProgramTitleEditor.h"
#if __has_include("ProgramTitleEditor.g.cpp")
#include "ProgramTitleEditor.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    ProgramTitleEditor::ProgramTitleEditor()
    {
        InitializeComponent();
    }

    int32_t ProgramTitleEditor::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ProgramTitleEditor::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void ProgramTitleEditor::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
