#pragma once

#include "DeviceResources.h"

enum class ShaderType
{
    Lights = 0,
    Toon = 1
};

class ShaderController
{
public:
    ShaderController::ShaderController(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Render();
    void ReleaseResources();
    void SetShader(ShaderType shaderType);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11PixelShader>       m_lightsPixelShader;
    winrt::com_ptr<ID3D11PixelShader>       m_toonPixelShader;

    bool                                    m_initialized;
    ShaderType                              m_currentShader;
};

