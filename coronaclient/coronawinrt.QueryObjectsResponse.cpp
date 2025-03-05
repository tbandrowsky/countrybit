#include "pch.h"
#include "coronawinrt.QueryObjectsResponse.h"
#include "coronawinrt.QueryObjectsResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> QueryObjectsResponse::Data()
    {
        throw hresult_not_implemented();
    }
    void QueryObjectsResponse::Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> const& value)
    {
        throw hresult_not_implemented();
    }
}
