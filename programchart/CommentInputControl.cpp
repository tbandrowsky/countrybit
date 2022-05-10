#include "pch.h"
#include "CommentInputControl.h"
#if __has_include("CommentInputControl.g.cpp")
#include "CommentInputControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    CommentInputControl::CommentInputControl()
    {
        InitializeComponent();
    }

    int32_t CommentInputControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void CommentInputControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void CommentInputControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
