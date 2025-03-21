#include "pch.h"
#include "CopyObjectPath.h"
#include "CopyObjectPath.g.cpp"


namespace winrt::coronawinrt::implementation
{
    hstring CopyObjectPath::ClassName()
    {
        return class_name;
    }
    void CopyObjectPath::ClassName(hstring const& value)
    {
        class_name = value;
    }
    hstring CopyObjectPath::Path()
    {
        return path;
    }
    void CopyObjectPath::Path(hstring const& value)
    {
        path = value;
    }
}
