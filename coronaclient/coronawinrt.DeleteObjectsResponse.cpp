#include "pch.h"
#include "coronawinrt.DeleteObjectsResponse.h"
#include "coronawinrt.DeleteObjectsResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject DeleteObjectsResponse::Data()
    {
        return data;
    }
    void DeleteObjectsResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
