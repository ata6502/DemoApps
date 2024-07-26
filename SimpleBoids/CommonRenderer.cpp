#include "pch.h"

#include <DirectXColors.h>

#include "CommonRenderer.h"
#include "FileReader.h"
#include "Utilities.h"

using namespace DirectX;

CommonRenderer::CommonRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false),
    m_cbufferNeverChanges(nullptr)
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

    // Create constant buffers.
    uint32_t byteWidth = (sizeof(CBufferNeverChanges) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbNeverChangesDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(&cbNeverChangesDesc, nullptr, m_cbufferNeverChanges.put()));
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

void CommonRenderer::PrepareRender()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Bind constant buffers.
    ID3D11Buffer* pCBufferNeverChanges{ m_cbufferNeverChanges.get() };

    context->PSSetConstantBuffers(0, 1, &pCBufferNeverChanges);
}

void CommonRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;

    m_cbufferNeverChanges = nullptr;
}

void CommonRenderer::SetLight(DirectionalLightDesc light)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferNeverChanges cbufferNeverChangesData;
    ZeroMemory(&cbufferNeverChangesData, sizeof(cbufferNeverChangesData));
    cbufferNeverChangesData.Light = light;

    // Copy the light to the constant buffer.
    context->UpdateSubresource(m_cbufferNeverChanges.get(), 0, nullptr, &cbufferNeverChangesData, 0, 0);
}

