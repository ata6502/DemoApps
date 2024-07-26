#pragma once

#include "ConstantBuffers.h"
#include "DeviceResources.h"

class CommonRenderer
{
public:
    CommonRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~CommonRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceResourcesAsync();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix);
    void PrepareRender();
    void ReleaseDeviceDependentResources();

    // Light methods.
    void SetLight(DirectionalLightDesc light);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    bool                                    m_initialized;

    winrt::com_ptr<ID3D11Buffer>            m_cbufferNeverChanges;
};

