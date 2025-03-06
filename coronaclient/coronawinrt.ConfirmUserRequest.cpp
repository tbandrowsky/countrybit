#include "pch.h"
#include "coronawinrt.ConfirmUserRequest.h"
#include "coronawinrt.ConfirmUserRequest.g.cpp"

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
