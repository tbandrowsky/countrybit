#include "pch.h"
#include "coronawinrt.QueryJoin.h"
#include "coronawinrt.QueryJoin.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryJoin::Source1()
    {
        return source1;
    }
    void QueryJoin::Source1(hstring const& value)
    {
        source1 = value;
    }
    hstring QueryJoin::Source2()
    {
        return source2;
    }
    void QueryJoin::Source2(hstring const& value)
    {
        source2 = value;
    }
    hstring QueryJoin::ResultName1()
    {
        return result_name1;
    }
    void QueryJoin::ResultName1(hstring const& value)
    {
        result_name1 = value;
    }
    hstring QueryJoin::ResultName2()
    {
        return result_name2;
    }
    void QueryJoin::ResultName2(hstring const& value)
    {
        result_name2 = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> QueryJoin::Keys()
    {
        return keys;
    }
    void QueryJoin::Keys(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        keys = value;
    }
}
