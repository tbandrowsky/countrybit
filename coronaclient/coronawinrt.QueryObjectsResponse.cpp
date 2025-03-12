#include "pch.h"
#include "coronawinrt.QueryObjectsResponse.h"
#include "coronawinrt.QueryObjectsResponse.g.cpp"

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
