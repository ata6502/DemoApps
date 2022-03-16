#include "pch.h"

#include "ShaderController.h"
#include "Utilities.h"

ShaderController::ShaderController(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false),
    m_currentShader(ShaderType::Lights)
{
    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction ShaderController::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Load pixel shader bytecode.
    auto pixelShaderBytecode1 = co_await Utilities::ReadDataAsync(L"LightsPixelShader.cso");
    auto pixelShaderBytecode2 = co_await Utilities::ReadDataAsync(L"ToonPixelShader.cso");

    // Create pixel shaders.
    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode1.data(),
            pixelShaderBytecode1.Length(),
            nullptr,
            m_lightsPixelShader.put()));

    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode2.data(),
            pixelShaderBytecode2.Length(),
            nullptr,
            m_toonPixelShader.put()));

    m_initialized = true;
}

void ShaderController::Render()
{
    if (!m_initialized)
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    switch (m_currentShader)
    {
    case ShaderType::Lights:
        context->PSSetShader(m_lightsPixelShader.get(), nullptr, 0);
        break;
    case ShaderType::Toon:
        context->PSSetShader(m_toonPixelShader.get(), nullptr, 0);
        break;
    }
}

void ShaderController::ReleaseResources()
{
    m_lightsPixelShader = nullptr;
    m_toonPixelShader = nullptr;
}

void ShaderController::SetShader(ShaderType shaderType)
{
    m_currentShader = shaderType;
}
