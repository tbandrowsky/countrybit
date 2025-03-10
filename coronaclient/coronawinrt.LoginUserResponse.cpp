#include "pch.h"
#include "coronawinrt.LoginUserResponse.h"
#include "coronawinrt.LoginUserResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaUser LoginUserResponse::Data()
    {
        return data;
    }
    void LoginUserResponse::Data(winrt::coronawinrt::CoronaUser const& value)
    {
        data = value;
    }
}
