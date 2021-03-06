#pragma once

#include "MainPage.g.h"
#include "DemoMain.h"

namespace winrt::SceneWithSkull::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        // Controls' event handlers.
        void ContentControl_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& args);
        void RendererListBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        void ScissorTestToggle_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
        void LeftRightMarginSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
        void TopBottomMarginSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
        void ThreeLightSystemRadioButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);

    private:
        // Window event handlers.
        void OnWindowLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
        void OnVisibilityChanged(winrt::Windows::UI::Core::CoreWindow const& sender, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args);

        // Other event handlers.
        void OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);

        // App live-cycle handlers.
        void OnSuspending(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::ApplicationModel::SuspendingEventArgs const&);
        void OnResuming(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);

        // Helper methods.
        void InitializePanels();
        void ToggleControlPanel();

        std::unique_ptr<DemoMain> m_main;
        bool m_controlPanelVisible;
        bool m_rendererInitialized;
    };
}

namespace winrt::SceneWithSkull::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
