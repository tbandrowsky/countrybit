#include "pch.h"
#include "coronawinrt.CoronaUser.h"
#include "coronawinrt.CoronaUser.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring CoronaUser::UserName()
    {
        return user_name;
    }
    void CoronaUser::UserName(hstring const& value)
    {
        user_name = value;
    }
    hstring CoronaUser::Email()
    {
        return email;
    }
    void CoronaUser::Email(hstring const& value)
    {
        email = value;
    }
    hstring CoronaUser::Address1()
    {
        return address1;
    }
    void CoronaUser::Address1(hstring const& value)
    {
        address1 = value;
    }
    hstring CoronaUser::Address2()
    {
        return address2;
    }
    void CoronaUser::Address2(hstring const& value)
    {
        address2 = value;
    }
    hstring CoronaUser::City()
    {
        return city;
    }
    void CoronaUser::City(hstring const& value)
    {
        city = value;
    }
    hstring CoronaUser::State()
    {
        return state;
    }
    void CoronaUser::State(hstring const& value)
    {
        state = value;
    }
    hstring CoronaUser::Zip()
    {
        return zip;
    }
    void CoronaUser::Zip(hstring const& value)
    {
        zip = value;
    }
}
