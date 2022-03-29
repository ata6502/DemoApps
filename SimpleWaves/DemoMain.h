#pragma once

#include "Camera.h"
#include "DeviceResources.h"
#include "IndependentInput.h"
#include "MaterialController.h"
#include "RasterizerStateManager.h"
#include "RendererBase.h"
#include "ShaderController.h"
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
    void SetShader(ShaderType shaderType);

    // Shader management.
    bool IsToonShaderSupported() const;

    // Rasterizer state management.
    void SetWireframeFillMode();
    void SetSolidFillMode();

    // Material management.
    void SetTerrainSpecularComponent(int specularComponent);
    void SetWaveSpecularComponent(int specularComponent);

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
    std::unique_ptr<ShaderController>        m_shaderController;
    std::unique_ptr<RasterizerStateManager>  m_rasterizerStateManager;
    std::string                              m_currentFillMode;
    std::shared_ptr<MaterialController>      m_materialController;
};

