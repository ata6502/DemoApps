#pragma once

#include "MainPage.g.h"
#include "DemoMain.h"

namespace winrt::HelloWorld::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

    private:
        // Window event handlers.
        void OnVisibilityChanged(winrt::Windows::UI::Core::CoreWindow const& sender, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args);

        // Other event handlers.
        void OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);

        // App live-cycle handlers.
        void OnSuspending(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::ApplicationModel::SuspendingEventArgs const&);
        void OnResuming(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);

        winrt::com_ptr<DemoMain> m_main;
    };
}

namespace winrt::HelloWorld::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
