#pragma once
#include "coronawinrt.CoronaBaseObject.g.h"


namespace winrt::coronawinrt::implementation
{
    struct CoronaBaseObject : CoronaBaseObjectT<CoronaBaseObject>
    {
        CoronaBaseObject() = default;

        hstring RawJson();
        void RawJson(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CoronaBaseObject : CoronaBaseObjectT<CoronaBaseObject, implementation::CoronaBaseObject>
    {
    };
}
