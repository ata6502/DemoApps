#include "pch.h"

#include "ThreeLightSystemController.h"
#include "Utilities.h"

ThreeLightSystemController::ThreeLightSystemController(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false),
    m_currentPixelShaderIndex(2)
{ 
    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::fire_and_forget ThreeLightSystemController::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Load pixel shader bytecode.
    auto pixelShaderBytecode1 = co_await Utilities::ReadDataAsync(L"LightsPixelShaderOneLight.cso");
    auto pixelShaderBytecode2 = co_await Utilities::ReadDataAsync(L"LightsPixelShaderTwoLights.cso");
    auto pixelShaderBytecode3 = co_await Utilities::ReadDataAsync(L"LightsPixelShaderThreeLights.cso");

    // Resize a vector to the number of pixel shaders.
    m_pixelShaders.resize(3);

    // Create pixel shaders and keep them in a vector.
    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode1.data(), 
            pixelShaderBytecode1.Length(), 
            nullptr, 
            m_pixelShaders[0].put()));

    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode2.data(), 
            pixelShaderBytecode2.Length(), 
            nullptr, 
            m_pixelShaders[1].put()));

    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode3.data(), 
            pixelShaderBytecode3.Length(), 
            nullptr, 
            m_pixelShaders[2].put()));

    m_initialized = true;
}

void ThreeLightSystemController::SetLightCount(int lightCount)
{
    ASSERT(lightCount >= 1 && lightCount <= 3);

    m_currentPixelShaderIndex = lightCount - 1;
}

void ThreeLightSystemController::Render()
{
    if (!m_initialized)
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->PSSetShader(m_pixelShaders[m_currentPixelShaderIndex].get(), nullptr, 0);
}

void ThreeLightSystemController::ReleaseResources()
{
    m_pixelShaders[0] = nullptr;
    m_pixelShaders[1] = nullptr;
    m_pixelShaders[2] = nullptr;
}