#include "pch.h"
#include "RunObjectResponse.h"
#include "RunObjectResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject RunObjectResponse::Data()
    {
        return data;
    }
    void RunObjectResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
