#include "pch.h"

#include <DirectXColors.h>

#include "CommonRenderer.h"
#include "FileReader.h"
#include "Utilities.h"

using namespace DirectX;

CommonRenderer::CommonRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false)
{
}

CommonRenderer::~CommonRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction CommonRenderer::CreateDeviceResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    using namespace std::literals::chrono_literals;
    co_await winrt::resume_after(1s);
}

// TODO: Remove FinalizeCreateDeviceResources if not needed.
// Create context-dependent resources.
void CommonRenderer::FinalizeCreateDeviceResources()
{
    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void CommonRenderer::CreateWindowSizeDependentResources()
{
    if (!m_initialized)
        return;
}

void CommonRenderer::Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix)
{
    if (!m_initialized)
        return;
}

void CommonRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;
}
