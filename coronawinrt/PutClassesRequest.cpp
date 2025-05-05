#include "pch.h"
#include "PutClassesRequest.h"
#include "PutClassesRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::ClassDefinition PutClassesRequest::ClassDef()
    {
        return class_def;
    }
    void PutClassesRequest::ClassDef(winrt::coronawinrt::ClassDefinition const& value)
    {
        class_def = value;
    }
}
