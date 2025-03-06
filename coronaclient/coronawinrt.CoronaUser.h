#pragma once
#include "coronawinrt.CoronaUser.g.h"
#include "coronawinrt.CoronaObject.h"

namespace winrt::coronawinrt::implementation
{
    struct CoronaUser : CoronaUserT<CoronaUser, coronawinrt::implementation::CoronaObject>
    {
        CoronaUser() = default;

        hstring user_name;
        hstring email;
        hstring address1;
        hstring address2;
        hstring city;
        hstring state;
        hstring zip;

        hstring UserName();
        void UserName(hstring const& value);
        hstring Email();
        void Email(hstring const& value);
        hstring Address1();
        void Address1(hstring const& value);
        hstring Address2();
        void Address2(hstring const& value);
        hstring City();
        void City(hstring const& value);
        hstring State();
        void State(hstring const& value);
        hstring Zip();
        void Zip(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CoronaUser : CoronaUserT<CoronaUser, implementation::CoronaUser>
    {
    };
}
