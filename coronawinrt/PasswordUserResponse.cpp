#include "pch.h"
#include "PasswordUserResponse.h"
#include "PasswordUserResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaUser PasswordUserResponse::Data()
    {
        return data;
    }
    void PasswordUserResponse::Data(winrt::coronawinrt::CoronaUser const& value)
    {
        data = value;
    }
}
