#include "pch.h"
#include "SpotlightConeHalfAngleConverter.h"
#include "SpotlightConeHalfAngleConverter.g.cpp"

namespace winrt::SimpleWaves::implementation
{
    Windows::Foundation::IInspectable SpotlightConeHalfAngleConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        auto angles = std::vector<hstring>{ L"8°", L"12°", L"16°", L"20°", L"30°", L"45°", L"60°", L"75°", L"90°" };
        auto index = static_cast<int>(value.as<double>());

        if (index >= angles.size())
            return winrt::box_value(hstring{ L"Undefined" });

        return winrt::box_value(angles[index]);
    }

    Windows::Foundation::IInspectable SpotlightConeHalfAngleConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}