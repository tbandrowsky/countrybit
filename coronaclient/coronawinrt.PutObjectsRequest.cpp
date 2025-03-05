#include "pch.h"
#include "coronawinrt.PutObjectsRequest.h"
#include "coronawinrt.PutObjectsRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> PutObjectsRequest::Data()
    {
        throw hresult_not_implemented();
    }
    void PutObjectsRequest::Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value)
    {
        throw hresult_not_implemented();
    }
}
