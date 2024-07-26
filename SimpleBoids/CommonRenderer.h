#pragma once

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
    void ReleaseDeviceDependentResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    bool                                    m_initialized;
};

