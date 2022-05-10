#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "TextInputControl.g.h"

namespace winrt::programchart::implementation
{
    struct TextInputControl : TextInputControlT<TextInputControl>
    {
        TextInputControl();

        winrt::hstring LabelText();
        LabelText(winrt::hstring _string);

        winrt::hstring TextValue();
        TextValue(winrt::hstring _string);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::programchart::factory_implementation
{
    struct TextInputControl : TextInputControlT<TextInputControl, implementation::TextInputControl>
    {
    };
}
