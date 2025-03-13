#include "pch.h"
#include "coronawinrt.SendUserRequest.h"
#include "coronawinrt.SendUserRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring SendUserRequest::Email()
    {
        return email;
    }
    void SendUserRequest::Email(hstring const& value)
    {
        email = value;
    }
}
