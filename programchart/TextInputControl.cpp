#include "pch.h"
#include "TextInputControl.h"
#if __has_include("TextInputControl.g.cpp")
#include "TextInputControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    TextInputControl::TextInputControl()
    {
        InitializeComponent();
    }


    void TextInputControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        
    }
}
