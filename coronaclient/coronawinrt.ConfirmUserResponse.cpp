#include "pch.h"
#include "coronawinrt.ConfirmUserResponse.h"
#include "coronawinrt.ConfirmUserResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaUser ConfirmUserResponse::Data()
    {
        return data;
    }
    void ConfirmUserResponse::Data(winrt::coronawinrt::CoronaUser const& value)
    {
        data = value;
    }
}
