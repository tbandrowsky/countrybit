#include "pch.h"
#include "SendUserRequest.h"
#include "SendUserRequest.g.cpp"

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
