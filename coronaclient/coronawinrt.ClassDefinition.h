#pragma once
#include "coronawinrt.ClassDefinition.g.h"


namespace winrt::coronawinrt::implementation
{
    struct ClassDefinition : ClassDefinitionT<ClassDefinition>
    {
        ClassDefinition() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
        hstring ClassDescription();
        void ClassDescription(hstring const& value);
        hstring BaseClassName();
        void BaseClassName(hstring const& value);
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
