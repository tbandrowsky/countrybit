#pragma once
#include "QueryJoin.g.h"
#include "QueryStage.h"

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

        hstring Source1();
        void Source1(hstring const& value);

        hstring Source2();
        void Source2(hstring const& value);

        hstring ResultName1();
        void ResultName1(hstring const& value);

        hstring ResultName2();
        void ResultName2(hstring const& value);

        winrt::Windows::Foundation::Collections::IVector<hstring> Keys();
        void Keys(winrt::Windows::Foundation::Collections::IVector<hstring>const& _keys);

    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryJoin : QueryJoinT<QueryJoin, implementation::QueryJoin>
    {
    };
}
