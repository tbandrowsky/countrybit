#include "pch.h"
#include "coronawinrt.ClassDefinition.h"
#include "coronawinrt.ClassDefinition.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring ClassDefinition::ClassName()
    {
        return class_name;
    }
    void ClassDefinition::ClassName(hstring const& value)
    {
        class_name = value;
    }
    hstring ClassDefinition::ClassDescription()
    {
        return class_description;
    }
    void ClassDefinition::ClassDescription(hstring const& value)
    {
        class_description = value;
    }
    hstring ClassDefinition::BaseClassName()
    {
        return base_class_name;
    }
    void ClassDefinition::BaseClassName(hstring const& value)
    {
        base_class_name = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::FieldDefinition> ClassDefinition::Fields()
    {
        return fields;
    }
    void ClassDefinition::Fields(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::FieldDefinition> const& value)
    {
        fields = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::IndexDefinition> ClassDefinition::Indexes()
    {
        return indexes;
    }
    void ClassDefinition::Indexes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::IndexDefinition> const& value)
    {
        indexes = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> ClassDefinition::DescendantClasses()
    {
        return descendant_classes;
    }
    void ClassDefinition::DescendantClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        descendant_classes = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> ClassDefinition::AncestorClasses()
    {
        return ancestor_classes;
    }
    void ClassDefinition::AncestorClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        ancestor_classes = value;
    }
}
