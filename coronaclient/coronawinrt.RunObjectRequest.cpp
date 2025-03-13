#include "pch.h"
#include "coronawinrt.RunObjectRequest.h"
#include "coronawinrt.RunObjectRequest.g.cpp"

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
