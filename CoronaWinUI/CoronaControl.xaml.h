#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "CoronaControl.g.h"

namespace winrt::CoronaWinUI::implementation
{
    struct CoronaControl : CoronaControlT<CoronaControl>
    {
        CoronaControl();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::CoronaWinUI::factory_implementation
{
    struct CoronaControl : CoronaControlT<CoronaControl, implementation::CoronaControl>
    {
    };
}
