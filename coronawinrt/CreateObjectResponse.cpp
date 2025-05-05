#include "pch.h"
#include "CreateObjectResponse.h"
#include "CreateObjectResponse.g.cpp"


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
