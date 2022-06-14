#pragma once

#include "DeviceResources.h"
#include "ShaderStructures.h"
#include "StepTimer.h"

class DemoMain : public winrt::implements<DemoMain, winrt::Windows::Foundation::IInspectable>, DX::IDeviceNotify
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

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

    Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

private:
    winrt::fire_and_forget InitializeInBackground();
    void Update();
    void Render();
    void ReleaseResources();

    std::shared_ptr<DX::DeviceResources>     m_deviceResources;

    // Direct3D resources for cube geometry.
    winrt::com_ptr<ID3D11InputLayout>        m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>             m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>             m_indexBuffer;
    winrt::com_ptr<ID3D11VertexShader>       m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>        m_pixelShader;
    winrt::com_ptr<ID3D11Buffer>             m_constantBuffer;

    ModelViewProjectionConstantBuffer        m_constantBufferData;
    uint32_t                                 m_indexCount;
    bool                                     m_initialized;
    DX::StepTimer                            m_timer;

    Concurrency::critical_section            m_criticalSection;
    winrt::Windows::Foundation::IAsyncAction m_renderLoopWorker;

    // The current rotation of the cube in radians.
    float                                    m_rotation;

    // Rotation speed in radians per second.
    float                                    m_rotationSpeed;
};

