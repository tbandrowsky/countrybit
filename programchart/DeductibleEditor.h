#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "DeductibleEditor.g.h"

namespace winrt::programchart::implementation
{
    struct DeductibleEditor : DeductibleEditorT<DeductibleEditor>
    {
        DeductibleEditor();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::programchart::factory_implementation
{
    struct DeductibleEditor : DeductibleEditorT<DeductibleEditor, implementation::DeductibleEditor>
    {
    };
}
