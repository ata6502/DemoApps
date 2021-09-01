#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Xaml;
using namespace Concurrency;

namespace winrt::TexturedCube::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();

        auto window = Window::Current().CoreWindow();
        window.VisibilityChanged({ this, &MainPage::OnVisibilityChanged });

        DisplayInformation::DisplayContentsInvalidated({ this, &MainPage::OnDisplayContentsInvalidated });

        DXSwapChainPanel().SizeChanged({ this, &MainPage::OnSwapChainPanelSizeChanged });
        DXSwapChainPanel().CompositionScaleChanged({ this, &MainPage::OnCompositionScaleChanged });

        Windows::UI::Xaml::Application::Current().Suspending({ this, &MainPage::OnSuspending });
        Windows::UI::Xaml::Application::Current().Resuming({ this, &MainPage::OnResuming });

        m_main = winrt::make_self<DemoMain>();
        m_main->SetSwapChainPanel(DXSwapChainPanel());
        m_main->StartRenderLoop();
    }

    void MainPage::OnVisibilityChanged([[maybe_unused]] winrt::Windows::UI::Core::CoreWindow const& sender, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args)
    {
        if (args.Visible())
            m_main->StartRenderLoop();
        else
            m_main->StopRenderLoop();
    }

    /// <summary>
    /// Occurs when the display requires redrawing.
    /// </summary>
    void MainPage::OnDisplayContentsInvalidated([[maybe_unused]] winrt::Windows::Graphics::Display::DisplayInformation const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        critical_section::scoped_lock lock(m_main->GetCriticalSection());
        m_main->ValidateDevice();
    }

    void MainPage::OnSwapChainPanelSizeChanged([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args)
    {
        critical_section::scoped_lock lock(m_main->GetCriticalSection());
        m_main->SetLogicalSize(args.NewSize());
        m_main->CreateWindowSizeDependentResources();
    }

    void MainPage::OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        critical_section::scoped_lock lock(m_main->GetCriticalSection());
        m_main->SetCompositionScale(sender.CompositionScaleX(), sender.CompositionScaleY());
        m_main->CreateWindowSizeDependentResources();
    }

    /// <summary>
    /// Occurs when the application transitions to Suspended state from some other state.
    /// Application state is saved without knowing whether the application will be terminated 
    /// or resumed.
    /// https://docs.microsoft.com/en-us/windows/uwp/launch-resume/suspend-an-app
    /// </summary>
    void MainPage::OnSuspending([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::ApplicationModel::SuspendingEventArgs const& args)
    {
        // Probably, we don't need critical section in Suspend at this point.
        critical_section::scoped_lock lock(m_main->GetCriticalSection());

        // Save application state and stop any background activity.
        m_main->Suspend();
    }

    /// <summary>
    /// Occurs when the application transitions from Suspended state to Running state.
    /// https://docs.microsoft.com/en-us/windows/uwp/launch-resume/resume-an-app
    /// </summary>
    void MainPage::OnResuming([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        // Load application state and resume any background activity.
        m_main->Resume();
    }
}
