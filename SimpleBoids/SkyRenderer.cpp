#include "pch.h"

#include <DirectXColors.h>

#include "FileReader.h"
#include "SkyRenderer.h"
#include "Utilities.h"

using namespace DirectX;

SkyRenderer::SkyRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false)
{
}

SkyRenderer::~SkyRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction SkyRenderer::CreateDeviceResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    using namespace std::literals::chrono_literals;
    co_await winrt::resume_after(1s);
}

// TODO: Remove FinalizeCreateDeviceResources if not needed.
// Create context-dependent resources.
void SkyRenderer::FinalizeCreateDeviceResources()
{
    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void SkyRenderer::CreateWindowSizeDependentResources()
{
    if (!m_initialized)
        return;
}

void SkyRenderer::Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix)
{
    if (!m_initialized)
        return;
}

void SkyRenderer::PrepareRender()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
}

void SkyRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;
}
