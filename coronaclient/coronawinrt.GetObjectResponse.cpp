#include "pch.h"
#include "coronawinrt.GetObjectResponse.h"
#include "coronawinrt.GetObjectResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject GetObjectResponse::Data()
    {
        return data;
    }
    void GetObjectResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
