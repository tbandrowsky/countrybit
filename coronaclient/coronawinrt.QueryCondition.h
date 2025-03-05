#pragma once
#include "coronawinrt.QueryCondition.g.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryCondition : QueryConditionT<QueryCondition>
    {
        QueryCondition() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryCondition : QueryConditionT<QueryCondition, implementation::QueryCondition>
    {
    };
}
