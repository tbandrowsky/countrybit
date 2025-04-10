#pragma once
#include "PutClassesRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct PutClassesRequest : PutClassesRequestT<PutClassesRequest, coronawinrt::implementation::BaseRequest>
    {
        winrt::coronawinrt::ClassDefinition class_def;

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
