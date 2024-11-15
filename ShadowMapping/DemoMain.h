#pragma once

#include "DeviceResources.h"
#include "IndependentInput.h"
#include "MainRenderer.h"
#include "StepTimer.h"

class DemoMain : public winrt::implements<DemoMain, winrt::Windows::Foundation::IInspectable>, public DX::IDeviceNotify
{
public:
    DemoMain();
    ~DemoMain();

    void FocusChanged(bool hasFocus);
    void DpiChanged(float logicalDpi);
    void OrientationChanged(winrt::Windows::Graphics::Display::DisplayOrientations currentOrientation);
    void ValidateDevice();

    void SwapChainPanelSizeChanged(winrt::Windows::Foundation::Size const& logicalSize);
    void SetCompositionScale(float compositionScaleX, float compositionScaleY);

    void Suspend();
    void Resume();

    void SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel);

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

private:
    std::shared_ptr<DX::DeviceResources>        m_deviceResources;

    std::unique_ptr<MainRenderer>               m_renderer;
    DX::StepTimer                               m_timer;
    Concurrency::critical_section               m_criticalSection;
    winrt::Windows::Foundation::IAsyncAction    m_renderLoopWorker;
    std::unique_ptr<IndependentInput>           m_input;
    bool                                        m_hasFocus;

    // Private helper methods.
    void StartRenderLoop();
    void StopRenderLoop();
    winrt::fire_and_forget Initialize();
    void CreateWindowSizeDependentResources();
    void Update();
};

