#include "pch.h"
#include "SendUserResponse.h"
#include "SendUserResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaUser SendUserResponse::Data()
    {
        return data;
    }
    void SendUserResponse::Data(winrt::coronawinrt::CoronaUser const& value)
    {
        data = value;
    }
}
