#include "pch.h"
#include "coronawinrt.LoginUserRequest.h"
#include "coronawinrt.LoginUserRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring LoginUserRequest::UserName()
    {
        return user_name;
    }
    void LoginUserRequest::UserName(hstring const& value)
    {
        user_name = value;
    }
    hstring LoginUserRequest::Password()
    {
        return password;
    }
    void LoginUserRequest::Password(hstring const& value)
    {
        password = value;
    }
}
