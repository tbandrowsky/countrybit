#include "pch.h"
#include "QueryObjectsResponse.h"
#include "QueryObjectsResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> QueryObjectsResponse::Data()
    {
        return data;
    }
    void QueryObjectsResponse::Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> const& value)
    {
        data = value;
    }
}
