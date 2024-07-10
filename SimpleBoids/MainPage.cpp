#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt::Windows::Graphics::Display; // DisplayInformation
using namespace winrt::Windows::UI::Core; // CoreWindowActivationState
using namespace winrt::Windows::UI::Input; // PointerVisualizationSettings
using namespace winrt::Windows::UI::Xaml; // Window
using namespace winrt::Windows::UI::Xaml::Controls; // ListBox
using namespace winrt::Windows::UI::Xaml::Media::Animation; // Storyboard

namespace winrt::SimpleBoids::implementation
{
    MainPage::MainPage() :
        m_controlPanelVisible(false)
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

        // SwapChainPanel event handlers.
        DXSwapChainPanel().SizeChanged({ this, &MainPage::OnSwapChainPanelSizeChanged });
        DXSwapChainPanel().CompositionScaleChanged({ this, &MainPage::OnCompositionScaleChanged });

        // Set SwapChainPanel in DeviceResources.
        m_main->SetSwapChainPanel(DXSwapChainPanel());

        BoidCountTextBlock().Text(std::to_wstring(m_main->GetSwarmSize()));
        MinimumDistanceSlider().Value(m_main->GetBoidMinDistance());
        MatchingFactorSlider().Value(m_main->GetBoidMatchingFactor());
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
    /// Occurs when either the CurrentOrientation or NativeOrientation property changes because of a mode change or a monitor change.
    /// The value of the CurrentOrientation property corresponds to the orientation of the display or monitor and not necessarily to 
    /// the orientation of your app. To determine the orientation of your app for layout purposes, use the ApplicationView.Value property.
    /// The value of the NativeOrientation property is typically the orientation where the buttons on the device match the orientation of the monitor.
    /// NativeOrientation returns only the Landscape or Portrait value. 
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
    /// 
    /// See remarks: https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.swapchainpanel
    /// 
    /// In order to maintain crisp vector rendering, you should listen for the CompositionScaleChanged event and query the CompositionScaleX and 
    /// CompositionScaleY property values to account for the current UI scale, and potentially render again from DirectX. Otherwise XAML layout 
    /// might do the scaling and your visuals might be degraded.
    /// </summary>
    void MainPage::OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, [[maybe_unused]] winrt::Windows::Foundation::IInspectable const& args)
    {
        m_main->SetCompositionScale(sender.CompositionScaleX(), sender.CompositionScaleY());
    }

    /// <summary>
    /// Occurs when the application transitions to Suspended state from some other state. Application state is saved without knowing whether 
    /// the application will be terminated or resumed.
    /// https://docs.microsoft.com/en-us/windows/uwp/launch-resume/suspend-an-app
    /// </summary>
    void MainPage::OnSuspending([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::ApplicationModel::SuspendingEventArgs const& args)
    {
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

    void MainPage::ContentControl_Tapped([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& args)
    {
        m_controlPanelVisible = !m_controlPanelVisible;

        auto resourceKey = winrt::box_value(m_controlPanelVisible ? L"ShowControlPanelStoryboard" : L"HideControlPanelStoryboard");
        auto storyboard = winrt::unbox_value<Storyboard>(this->Resources().Lookup(resourceKey));
        storyboard.Begin();
    }

    void MainPage::RestartSimulationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        m_main->RestartSimulation();
    }

    void MainPage::AddBoidsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        m_main->AddBoids();
        BoidCountTextBlock().Text(std::to_wstring(m_main->GetSwarmSize()));
    }

    void MainPage::RemoveBoidsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        m_main->RemoveBoids();
        BoidCountTextBlock().Text(std::to_wstring(m_main->GetSwarmSize()));
    }

    void MainPage::BoidShapeListBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args)
    {
        auto listBox = sender.as<ListBox>();
        if (!listBox.IsLoaded())
            return;

        auto boidShapeIndex = listBox.SelectedIndex();
        m_main->SetBoidShape(boidShapeIndex);
    }

    void MainPage::MinimumDistanceSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (!MinimumDistanceSlider().IsLoaded())
            return;

        float boidMinDistance = static_cast<float>(MinimumDistanceSlider().Value());
        m_main->SetBoidMinDistance(boidMinDistance);
    }

    void MainPage::MatchingFactorSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (!MatchingFactorSlider().IsLoaded())
            return;

        float boidMatchingFactor = static_cast<float>(MatchingFactorSlider().Value());
        m_main->SetBoidMatchingFactor(boidMatchingFactor);
    }
}
