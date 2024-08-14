#pragma once

#include "ConstantBuffers.h"
#include "DeviceResources.h"

class CommonRenderer
{
public:
    CommonRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~CommonRenderer();

    void CreateDeviceResources();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix);
    void PrepareRender();
    void ReleaseDeviceDependentResources();

    void SetLight(DirectionalLightDesc light);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    bool                                    m_initialized;

    winrt::com_ptr<ID3D11Buffer>            m_cbufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferOnResize;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferPerFrame;
    winrt::com_ptr<ID3D11SamplerState>      m_linearSampler;
};

