#pragma once
#include "coronawinrt.CopyObjectPath.g.h"

namespace winrt::coronawinrt::implementation
{
    struct CopyObjectPath : CopyObjectPathT<CopyObjectPath>
    {
        hstring class_name;
        hstring path;

        CopyObjectPath() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
        hstring Path();
        void Path(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CopyObjectPath : CopyObjectPathT<CopyObjectPath, implementation::CopyObjectPath>
    {
    };
}
