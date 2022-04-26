#pragma once

#include "SpotlightConeHalfAngleConverter.g.h"

namespace winrt::SimpleWaves::implementation
{
    struct SpotlightConeHalfAngleConverter : SpotlightConeHalfAngleConverterT<SpotlightConeHalfAngleConverter>
    {
        SpotlightConeHalfAngleConverter() = default;

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}

namespace winrt::SimpleWaves::factory_implementation
{
    struct SpotlightConeHalfAngleConverter : SpotlightConeHalfAngleConverterT<SpotlightConeHalfAngleConverter, implementation::SpotlightConeHalfAngleConverter>
    {
    };
}
