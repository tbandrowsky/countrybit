#pragma once
#include "coronawinrt.QueryJoin.g.h"
#include "coronawinrt.QueryStage.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryJoin : QueryJoinT<QueryJoin, coronawinrt::implementation::QueryStage>
    {
        QueryJoin() = default;

        hstring Source1();
        void Source1(hstring const& value);
        hstring Source2();
        void Source2(hstring const& value);
        hstring ResultName1();
        void ResultName1(hstring const& value);
        hstring ResultName2();
        void ResultName2(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> Keys();
        void Keys(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryJoin : QueryJoinT<QueryJoin, implementation::QueryJoin>
    {
    };
}
