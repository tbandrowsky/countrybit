#include "pch.h"
#include "PasswordUserRequest.h"
#include "PasswordUserRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring PasswordUserRequest::UserName()
    {
        return user_name;
    }
    void PasswordUserRequest::UserName(hstring const& value)
    {
        user_name = value;
    }
    hstring PasswordUserRequest::ValidationCode()
    {
        return validation_code;
    }
    void PasswordUserRequest::ValidationCode(hstring const& value)
    {
        validation_code = value;
    }
    hstring PasswordUserRequest::Password1()
    {
        return password1;
    }
    void PasswordUserRequest::Password1(hstring const& value)
    {
        password1 = value;
    }
    hstring PasswordUserRequest::Password2()
    {
        return password2;
    }
    void PasswordUserRequest::Password2(hstring const& value)
    {
        password2 = value;
    }
}
