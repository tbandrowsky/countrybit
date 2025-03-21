#include "pch.h"
#include "DeleteObjectsResponse.h"
#include "DeleteObjectsResponse.g.cpp"

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
