#include "pch.h"
#include "CreateUserResponse.h"
#include "CreateUserResponse.g.cpp"


namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaUser CreateUserResponse::Data()
    {
        return data;
    }
    void CreateUserResponse::Data(winrt::coronawinrt::CoronaUser const& value)
    {
        data = value;
    }
}
