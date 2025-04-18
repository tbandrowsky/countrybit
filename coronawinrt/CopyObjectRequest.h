#pragma once
#include "CopyObjectRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct CopyObjectRequest : CopyObjectRequestT<CopyObjectRequest, coronawinrt::implementation::BaseRequest>
    {
        CopyObjectPath source;
        CopyObjectPath dest;
        CopyObjectPath transform;

        CopyObjectRequest() = default;

        winrt::coronawinrt::CopyObjectPath Source();
        void Source(winrt::coronawinrt::CopyObjectPath const& value);
        winrt::coronawinrt::CopyObjectPath Dest();
        void Dest(winrt::coronawinrt::CopyObjectPath const& value);
        winrt::coronawinrt::CopyObjectPath Transform();
        void Transform(winrt::coronawinrt::CopyObjectPath const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CopyObjectRequest : CopyObjectRequestT<CopyObjectRequest, implementation::CopyObjectRequest>
    {
    };
}
