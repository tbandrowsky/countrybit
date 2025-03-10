#include "pch.h"
#include "coronawinrt.EditObjectResponse.h"
#include "coronawinrt.EditObjectResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject EditObjectResponse::Data()
    {
        return data;
    }
    void EditObjectResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
