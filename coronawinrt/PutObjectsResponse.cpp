#include "pch.h"
#include "PutObjectsResponse.h"
#include "PutObjectsResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> PutObjectsResponse::Data()
    {
        return data;
    }
    void PutObjectsResponse::Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value)
    {
        data = value;
    }
}
