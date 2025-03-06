#include "pch.h"
#include "coronawinrt.CreateObjectResponse.h"
#include "coronawinrt.CreateObjectResponse.g.cpp"


namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject CreateObjectResponse::Data()
    {
        return data;
    }
    void CreateObjectResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
