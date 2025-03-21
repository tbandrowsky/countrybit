#pragma once
#include "QueryProjectField.g.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryProjectField : QueryProjectFieldT<QueryProjectField>
    {
        QueryProjectField() = default;

        hstring FieldName();
        void FieldName(hstring const& value);
        hstring ValuePath();
        void ValuePath(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryProjectField : QueryProjectFieldT<QueryProjectField, implementation::QueryProjectField>
    {
    };
}
