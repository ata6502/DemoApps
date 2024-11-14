#include "pch.h"

#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt::Windows::Graphics::Display; // DisplayInformation
using namespace winrt::Windows::UI::Core; // CoreWindowActivationState
using namespace winrt::Windows::UI::Input; // PointerVisualizationSettings
using namespace winrt::Windows::UI::Xaml; // Window

namespace winrt::ShadowMapping::implementation
{
    MainPage::MainPage()
    {
        // Window event handlers.
        auto window = Window::Current().CoreWindow();
        window.Activated({ this, &MainPage::OnWindowActivationChanged });

        // DisplayInformation event handlers.
        DisplayInformation displayInformation{ DisplayInformation::GetForCurrentView() };
        displayInformation.DpiChanged({ this, &MainPage::OnDpiChanged });
        displayInformation.OrientationChanged({ this, &MainPage::OnOrientationChanged });
        DisplayInformation::DisplayContentsInvalidated({ this, &MainPage::OnDisplayContentsInvalidated });

        // App live-cycle handlers.
        Windows::UI::Xaml::Application::Current().Suspending({ this, &MainPage::OnSuspending });
        Windows::UI::Xaml::Application::Current().Resuming({ this, &MainPage::OnResuming });

        // Disable all pointer visual feedback for better performance when touching.
        auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
        pointerVisualizationSettings.IsContactFeedbackEnabled(false);
        pointerVisualizationSettings.IsBarrelButtonFeedbackEnabled(false);

        m_main = winrt::make_self<DemoMain>();
    }

    void MainPage::InitializeComponent()
    {
        // Call the base class InitializeComponent to register with the Xaml runtime.
        MainPageT::InitializeComponent();

        // Now, we can access Xaml properties.

        // SwapChainPanel event handlers.
        DXSwapChainPanel().SizeChanged({ this, &MainPage::OnSwapChainPanelSizeChanged });
        DXSwapChainPanel().CompositionScaleChanged({ this, &MainPage::OnCompositionScaleChanged });

        m_main->SetSwapChainPanel(DXSwapChainPanel()); // set SwapChainPanel in DeviceResources
    }

    /// <summary>
    /// Occurs when the window completes activation or deactivation.
    /// </summary>
    void MainPage::OnWindowActivationChanged([[maybe_unused]] winrt::Windows::UI::Core::CoreWindow const& sender, winrt::Windows::UI::Core::WindowActivatedEventArgs const& args)
    {
        if (args.WindowActivationState() == CoreWindowActivationState::Deactivated)
        {
            m_main->FocusChanged(false);
        }
        else if (args.WindowActivationState() == CoreWindowActivationState::CodeActivated
            || args.WindowActivationState() == CoreWindowActivationState::PointerActivated)
        {
            m_main->FocusChanged(true);
        }
    }

    /// <summary>
    /// Occurs when the LogicalDpi property changes because the pixels per inch (PPI) of the display changed.
    /// </summary>
    void MainPage::OnDpiChanged([[maybe_unused]] winrt::Windows::Graphics::Display::DisplayInformation const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        m_main->DpiChanged(sender.LogicalDpi());
    }

    /// <summary>
    /// Occurs when either the CurrentOrientation or NativeOrientation property changes.
    /// </summary>
    void MainPage::OnOrientationChanged([[maybe_unused]] winrt::Windows::Graphics::Display::DisplayInformation const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        m_main->OrientationChanged(sender.CurrentOrientation());
    }

    /// <summary>
    /// Occurs when the display requires redrawing, for example, when the user changes the size of text (DPI).
    /// </summary>
    void MainPage::OnDisplayContentsInvalidated([[maybe_unused]] winrt::Windows::Graphics::Display::DisplayInformation const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        m_main->ValidateDevice();
    }

    /// <summary>
    /// Occurs when either the ActualHeight or the ActualWidth property changes value.
    /// </summary>
    void MainPage::OnSwapChainPanelSizeChanged([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args)
    {
        m_main->SwapChainPanelSizeChanged(args.NewSize());
    }

    /// <summary>
    /// Occurs when the composition scale factor of the SwapChainPanel has changed.
    /// </summary>
    void MainPage::OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        m_main->SetCompositionScale(sender.CompositionScaleX(), sender.CompositionScaleY());
    }

    /// <summary>
    /// Occurs when the application transitions to Suspended state from some other state.
    /// </summary>
    void MainPage::OnSuspending([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::ApplicationModel::SuspendingEventArgs const& args)
    {
        m_main->Suspend();
    }

    /// <summary>
    /// Occurs when the application transitions from Suspended state to Running state.
    /// </summary>
    void MainPage::OnResuming([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        m_main->Resume();
    }
}
