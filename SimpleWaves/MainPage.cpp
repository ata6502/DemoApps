#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;            // ListBox
using namespace Windows::UI::Xaml::Media::Animation;    // Storyboard
using namespace Concurrency;

namespace winrt::SimpleWaves::implementation
{
    MainPage::MainPage() :
        m_main(nullptr),
        m_controlPanelVisible(false)
    {
        InitializeComponent();

        Loaded({ this, &MainPage::OnWindowLoaded });

        auto window = Window::Current().CoreWindow();
        window.VisibilityChanged({ this, &MainPage::OnVisibilityChanged });

        DisplayInformation::DisplayContentsInvalidated({ this, &MainPage::OnDisplayContentsInvalidated });

        DXSwapChainPanel().SizeChanged({ this, &MainPage::OnSwapChainPanelSizeChanged });
        DXSwapChainPanel().CompositionScaleChanged({ this, &MainPage::OnCompositionScaleChanged });

        Windows::UI::Xaml::Application::Current().Suspending({ this, &MainPage::OnSuspending });
        Windows::UI::Xaml::Application::Current().Resuming({ this, &MainPage::OnResuming });

        m_main = std::make_unique<DemoMain>();
        m_main->SetSwapChainPanel(DXSwapChainPanel());
        m_main->StartRenderLoop();
    }

    void MainPage::OnWindowLoaded([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        InitializePanels();
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

    void MainPage::ContentControl_Tapped([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& args)
    {
        ToggleControlPanel();
    }

    void MainPage::ToggleControlPanel()
    {
        m_controlPanelVisible = !m_controlPanelVisible;

        auto resourceKey = winrt::box_value(m_controlPanelVisible ? L"ShowControlPanelStoryboard" : L"HideControlPanelStoryboard");
        auto storyboard = winrt::unbox_value<Storyboard>(this->Resources().Lookup(resourceKey));
        storyboard.Begin();
    }

    void MainPage::RendererListBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args)
    {
        if (m_main == nullptr)
            return;

        auto listBox = sender.as<ListBox>();
        auto selectedIndex = listBox.SelectedIndex();
        m_main->SetRenderer(selectedIndex);

        InitializePanels();
        SetShader();
    }

    void MainPage::ToonShaderToggle_Toggled([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        critical_section::scoped_lock lock(m_main->GetCriticalSection());

        SetShader();
    }

    void MainPage::WireframeToggle_Toggled([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        critical_section::scoped_lock lock(m_main->GetCriticalSection());

        if (WireframeToggle().IsOn())
            m_main->SetWireframeFillMode();
        else
            m_main->SetSolidFillMode();
    }

    void MainPage::SpecularComponentSlider_ValueChanged([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (m_main == nullptr)
            return;

        // Set specular component of terrain and waves.
        m_main->SetSpecularComponent(static_cast<int>(args.NewValue()));
    }

    void MainPage::SpotlightConeHalfAngleSlider_ValueChanged([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (m_main == nullptr)
            return;

        auto halfAngleIndex = static_cast<int>(args.NewValue());
        m_main->SetSpotlightConeHalfAngle(halfAngleIndex);
    }

    void MainPage::FogStartSlider_ValueChanged([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (m_main == nullptr)
            return;

        m_main->SetFogStart(static_cast<float>(args.NewValue()));
    }


    void MainPage::FogRangeSlider_ValueChanged([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (m_main == nullptr)
            return;

        m_main->SetFogRange(static_cast<float>(args.NewValue()));
    }

    void MainPage::InitializePanels()
    {
        if (m_main->IsToonShaderSupported())
            ToonShaderGrid().Visibility(Visibility::Visible);
        else
            ToonShaderGrid().Visibility(Visibility::Collapsed);

        if (m_main->AreLightParametersSupported())
            LightControlPanel().Visibility(Visibility::Visible);
        else
            LightControlPanel().Visibility(Visibility::Collapsed);

        if (m_main->IsFogSupported())
            FogControlPanel().Visibility(Visibility::Visible);
        else
            FogControlPanel().Visibility(Visibility::Collapsed);
    }

    void MainPage::SetShader()
    {
        if (m_main == nullptr)
            return;

        auto isToonShaderEnabled = ToonShaderToggle().IsOn();

        if (m_main->IsToonShaderSupported() && isToonShaderEnabled)
            m_main->SetShader(ShaderType::Toon); 
        else
            m_main->SetShader(ShaderType::Lights);
    }
}
