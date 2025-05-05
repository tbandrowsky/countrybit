#pragma once
#include "FieldOptions.g.h"

namespace winrt::coronawinrt::implementation
{
    struct FieldOptions : FieldOptionsT<FieldOptions>
    {
        FieldOptions() = default;

    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct FieldOptions : FieldOptionsT<FieldOptions, implementation::FieldOptions>
    {
    };
}
