#include "pch.h"
#include "CoronaSliceControl.h"
#if __has_include("CoronaSliceControl.g.cpp")
#include "CoronaSliceControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::programchart::implementation
{
    CoronaSliceControl::CoronaSliceControl()
    {
        InitializeComponent();
    }

    int64_t CoronaSliceControl::ObjectId()
    {
        ;
    }

    void CoronaSliceControl::ObjectId(int64_t _objectId)
    {
        ;
    }

    void CoronaSliceControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
