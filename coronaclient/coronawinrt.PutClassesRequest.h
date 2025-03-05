#pragma once
#include "coronawinrt.PutClassesRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct PutClassesRequest : PutClassesRequestT<PutClassesRequest, coronawinrt::implementation::BaseRequest>
    {
        PutClassesRequest() = default;

        winrt::coronawinrt::ClassDefinition ClassDef();
        void ClassDef(winrt::coronawinrt::ClassDefinition const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct PutClassesRequest : PutClassesRequestT<PutClassesRequest, implementation::PutClassesRequest>
    {
    };
}
