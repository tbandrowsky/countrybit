#pragma once
#include "coronawinrt.DoubleFieldOptions.g.h"
#include "coronawinrt.FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct DoubleFieldOptions : DoubleFieldOptionsT<DoubleFieldOptions, coronawinrt::implementation::FieldOptions>
    {
        DoubleFieldOptions() = default;

        double min_value;
        double max_value;
        hstring message;

        double MinValue();
        void MinValue(double value);
        double MaxValue();
        void MaxValue(double value);
        hstring Message();
        void Message(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct DoubleFieldOptions : DoubleFieldOptionsT<DoubleFieldOptions, implementation::DoubleFieldOptions>
    {
    };
}
