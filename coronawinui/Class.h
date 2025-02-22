#pragma once

#include "Class.g.h"

namespace winrt::coronawinui::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::coronawinui::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
