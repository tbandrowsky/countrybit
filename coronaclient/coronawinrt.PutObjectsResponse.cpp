#include "pch.h"
#include "coronawinrt.PutObjectsResponse.h"
#include "coronawinrt.PutObjectsResponse.g.cpp"

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
