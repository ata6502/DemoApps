#pragma once

#include "DeviceResources.h"

class SkyRenderer
{
public:
    SkyRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~SkyRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceResourcesAsync();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix);
    void PrepareRender();
    void ReleaseDeviceDependentResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    bool                                    m_initialized;
};

