#pragma once
#include "ClassDefinition.g.h"

namespace winrtf = winrt::Windows::Foundation::Collections;

namespace winrt::coronawinrt::implementation
{
    struct ClassDefinition : ClassDefinitionT<ClassDefinition>
    {
        ClassDefinition() = default;

        hstring class_name;
        hstring class_description;
        hstring base_class_name;
        winrtf::IVector<winrt::coronawinrt::FieldDefinition> fields;
        winrtf::IVector<winrt::coronawinrt::IndexDefinition> indexes;
        winrtf::IVector<hstring> descendant_classes;
        winrtf::IVector<hstring> ancestor_classes;
        winrtf::IVector<hstring> parents;

        hstring ClassName();
        void ClassName(hstring const& value);
        hstring ClassDescription();
        void ClassDescription(hstring const& value);
        hstring BaseClassName();
        void BaseClassName(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> Parents();
        void Parents(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::FieldDefinition> Fields();
        void Fields(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::FieldDefinition> const& value);
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::IndexDefinition> Indexes();
        void Indexes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::IndexDefinition> const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> DescendantClasses();
        void DescendantClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> AncestorClasses();
        void AncestorClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct ClassDefinition : ClassDefinitionT<ClassDefinition, implementation::ClassDefinition>
    {
    };
}
