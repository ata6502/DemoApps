#pragma once

#include "DemoMain.h"
#include "MainPage.g.h"

namespace winrt::SimpleBoids::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        void InitializeComponent();
        void ContentControl_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& args);
        void RestartSimulationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
        void AddBoidsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
        void RemoveBoidsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
        void BoidShapeListBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        void BoidParameterChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);

    private:
        // Window event handlers.
        void OnWindowActivationChanged(winrt::Windows::UI::Core::CoreWindow const& sender, winrt::Windows::UI::Core::WindowActivatedEventArgs const& args);

        // DisplayInformation event handlers.
        void OnDpiChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OnOrientationChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);

        // SwapChainPanel event handlers.
        void OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, winrt::Windows::Foundation::IInspectable const& args);

        // App live-cycle handlers.
        void OnSuspending(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::ApplicationModel::SuspendingEventArgs const&);
        void OnResuming(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);

        winrt::com_ptr<DemoMain> m_main;
        bool m_controlPanelVisible;
    };
}

namespace winrt::SimpleBoids::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
