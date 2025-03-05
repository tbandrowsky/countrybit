#include "pch.h"
#include "coronawinrt.PutObjectsResponse.h"
#include "coronawinrt.PutObjectsResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> PutObjectsResponse::Data()
    {
        throw hresult_not_implemented();
    }
    void PutObjectsResponse::Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value)
    {
        throw hresult_not_implemented();
    }
}
