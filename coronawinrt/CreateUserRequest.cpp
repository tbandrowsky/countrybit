#include "pch.h"
#include "CreateUserRequest.h"
#include "CreateUserRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring CreateUserRequest::Username()
    {
        return username;;
    }
    void CreateUserRequest::Username(hstring const& value)
    {
        username = value;
    }
    hstring CreateUserRequest::Email()
    {
        return email;
    }
    void CreateUserRequest::Email(hstring const& value)
    {
        email = value;
    }
    hstring CreateUserRequest::Address1()
    {
        return address1;
    }
    void CreateUserRequest::Address1(hstring const& value)
    {
        address1 = value;
    }
    hstring CreateUserRequest::Address2()
    {
        return address2;
    }
    void CreateUserRequest::Address2(hstring const& value)
    {
        address2 = value;
    }
    hstring CreateUserRequest::City()
    {
        return city;
    }
    void CreateUserRequest::City(hstring const& value)
    {
        city = value;
    }
    hstring CreateUserRequest::State()
    {
        return state;
    }
    void CreateUserRequest::State(hstring const& value)
    {
        state = value;
    }
    hstring CreateUserRequest::Zip()
    {
        return zip;
    }
    void CreateUserRequest::Zip(hstring const& value)
    {
        zip = value;
    }
    hstring CreateUserRequest::Password1()
    {
        return password1;
    }
    void CreateUserRequest::Password1(hstring const& value)
    {
        password1 = value;
    }
    hstring CreateUserRequest::Password2()
    {
        return password2;
    }
    void CreateUserRequest::Password2(hstring const& value)
    {
        password2 = value;
    }
}
