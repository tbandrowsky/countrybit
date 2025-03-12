#pragma once
#include "coronawinrt.QueryJoin.g.h"
#include "coronawinrt.QueryStage.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryJoin : QueryJoinT<QueryJoin, coronawinrt::implementation::QueryStage>
    {
        QueryJoin() = default;

        hstring source1;
        hstring source2;
        hstring result_name1;
        hstring result_name2;
        winrt::Windows::Foundation::Collections::IVector<hstring> keys;
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryJoin : QueryJoinT<QueryJoin, implementation::QueryJoin>
    {
    };
}
