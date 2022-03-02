#pragma once

#include "DeviceResources.h"

class ThreeLightSystemController
{
public:
    ThreeLightSystemController::ThreeLightSystemController(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Render();
    void ReleaseResources();
    void SetLightCount(int lightCount);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    std::vector<winrt::com_ptr<ID3D11PixelShader>> m_pixelShaders;

    uint32_t                                m_currentPixelShaderIndex;
    bool                                    m_initialized;
};

