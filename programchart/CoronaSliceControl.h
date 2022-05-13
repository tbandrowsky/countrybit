#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "CoronaSliceControl.g.h"

namespace winrt::programchart::implementation
{
    struct CoronaSliceControl : CoronaSliceControlT<CoronaSliceControl>
    {
        CoronaSliceControl();

        int64_t ObjectId();
        void ObjectId(int64_t _objectId);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::programchart::factory_implementation
{
    struct CoronaSliceControl : CoronaSliceControlT<CoronaSliceControl, implementation::CoronaSliceControl>
    {
    };
}
