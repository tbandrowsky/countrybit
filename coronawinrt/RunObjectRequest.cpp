#include "pch.h"
#include "RunObjectRequest.h"
#include "RunObjectRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject RunObjectRequest::Data()
    {
        return data;
    }
    void RunObjectRequest::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
