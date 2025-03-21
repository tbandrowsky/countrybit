#include "pch.h"
#include "PutObjectsRequest.h"
#include "PutObjectsRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> PutObjectsRequest::Data()
    {
        return data;
    }
    void PutObjectsRequest::Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value)
    {
        data = value;
    }
}
