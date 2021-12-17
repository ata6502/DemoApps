#pragma once

#include "Camera.h"
#include "DeviceResources.h"
#include "IndependentInput.h"
#include "RendererBase.h"
#include "Timer.h"

class DemoMain : public DX::IDeviceNotify
{
public:
    DemoMain();
    ~DemoMain();

    void CreateWindowSizeDependentResources();

    void StartRenderLoop();
    void StopRenderLoop();

    void SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel);
    void SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize);
    void SetCompositionScale(float compositionScaleX, float compositionScaleY);

    void ValidateDevice();

    void Suspend();
    void Resume();

    void SetRenderer(int32_t rendererIndex);

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

    Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

private:
    void Update();
    void Render();
    void ReleaseResources();

    std::shared_ptr<DX::DeviceResources>     m_deviceResources;
    Timer                                    m_timer;
    Concurrency::critical_section            m_criticalSection;
    winrt::Windows::Foundation::IAsyncAction m_renderLoopWorker;
    std::unique_ptr<IndependentInput>        m_input;
    std::unique_ptr<Camera>                  m_camera;
    std::unique_ptr<RendererBase>            m_renderer;
};

