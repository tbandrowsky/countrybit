#include "pch.h"
#include "ConfirmUserRequest.h"
#include "ConfirmUserRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring ConfirmUserRequest::Email()
    {
        return email;
    }
    void ConfirmUserRequest::Email(hstring const& value)
    {
        email = value;
    }
    hstring ConfirmUserRequest::ValidationCode()
    {
        return validation_code;
    }
    void ConfirmUserRequest::ValidationCode(hstring const& value)
    {
        validation_code = value;
    }
}
