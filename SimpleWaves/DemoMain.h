#pragma once

#include "Camera.h"
#include "DeviceResources.h"
#include "FogController.h"
#include "IndependentInput.h"
#include "LightsController.h"
#include "MaterialController.h"
#include "RendererBase.h"
#include "ShaderController.h"
#include "StateManager.h"
#include "StepTimer.h"

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

    // Shader control.
    bool IsToonShaderSupported() const;
    bool AreLightParametersSupported() const;

    // Rasterizer state control.
    void SetWireframeFillMode();
    void SetSolidFillMode();

    // Material control.
    void SetSpecularComponent(int specularComponent);

    // Light control.
    void SetSpotlightConeHalfAngle(int halfAngleIndex);

    // Fog control.
    bool IsFogSupported() const;
    void SetFogStart(float fogStart);
    void SetFogRange(float fogRange);

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

    Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

private:
    void Update();
    void Render();
    void ReleaseResources();

    std::shared_ptr<DX::DeviceResources>     m_deviceResources;
    DX::StepTimer                            m_timer;
    Concurrency::critical_section            m_criticalSection;
    winrt::Windows::Foundation::IAsyncAction m_renderLoopWorker;
    std::unique_ptr<IndependentInput>        m_input;
    std::unique_ptr<Camera>                  m_camera;
    std::unique_ptr<RendererBase>            m_renderer;
    std::unique_ptr<StateManager>            m_stateManager;
    std::string                              m_currentFillMode;

    // Controllers
    std::unique_ptr<ShaderController>        m_shaderController;
    std::shared_ptr<MaterialController>      m_materialController;
    std::shared_ptr<LightsController>        m_lightsController;
    std::shared_ptr<FogController>           m_fogController;
};

