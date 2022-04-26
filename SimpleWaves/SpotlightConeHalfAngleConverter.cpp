#include "pch.h"
#include "SpotlightConeHalfAngleConverter.h"
#include "SpotlightConeHalfAngleConverter.g.cpp"

namespace winrt::SimpleWaves::implementation
{
    Windows::Foundation::IInspectable SpotlightConeHalfAngleConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        auto val = static_cast<int>(value.as<double>());
        switch (val)
        {
        case 1:
            return winrt::box_value(hstring{ L"8°" });
        case 2:
            return winrt::box_value(hstring{ L"12°" });
        case 3:
            return winrt::box_value(hstring{ L"16°" });
        case 4:
            return winrt::box_value(hstring{ L"20°" });
        case 5:
            return winrt::box_value(hstring{ L"30°" });
        case 6:
            return winrt::box_value(hstring{ L"45°" });
        case 7:
            return winrt::box_value(hstring{ L"60°" });
        case 8:
            return winrt::box_value(hstring{ L"75°" });
        case 9:
            return winrt::box_value(hstring{ L"90°" });
        default:
            return winrt::box_value(hstring{ L"Undefined" });
        }
    }

    Windows::Foundation::IInspectable SpotlightConeHalfAngleConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}