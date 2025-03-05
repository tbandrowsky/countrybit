#pragma once
#include "coronawinrt.CoronaObject.g.h"
#include "coronawinrt.CoronaBaseObject.h"


namespace winrt::coronawinrt::implementation
{
    struct CoronaObject : CoronaObjectT<CoronaObject, coronawinrt::implementation::CoronaBaseObject>
    {
        CoronaObject() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
        int64_t ObjectId();
        void ObjectId(int64_t value);
        winrt::Windows::Foundation::DateTime Created();
        void Created(winrt::Windows::Foundation::DateTime const& value);
        hstring CreatedBy();
        void CreatedBy(hstring const& value);
        winrt::Windows::Foundation::DateTime Updated();
        void Updated(winrt::Windows::Foundation::DateTime const& value);
        hstring UpdatedBy();
        void UpdatedBy(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CoronaObject : CoronaObjectT<CoronaObject, implementation::CoronaObject>
    {
    };
}
