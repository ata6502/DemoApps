#include "pch.h"

#include <DirectXColors.h>

#include "CommonRenderer.h"
#include "FileReader.h"
#include "Utilities.h"

using namespace DirectX;

CommonRenderer::CommonRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false),
    m_cbufferNeverChanges(nullptr),
    m_cbufferOnResize(nullptr),
    m_cbufferPerFrame(nullptr),
    m_linearSampler(nullptr)
{
}

CommonRenderer::~CommonRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
void CommonRenderer::CreateDeviceResources()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Create constant buffers.
    uint32_t byteWidth = (sizeof(CBufferNeverChanges) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbNeverChangesDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(&cbNeverChangesDesc, nullptr, m_cbufferNeverChanges.put()));

    byteWidth = (sizeof(CBufferOnResize) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbOnResizeDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(device->CreateBuffer(&cbOnResizeDesc, nullptr, m_cbufferOnResize.put()));

    byteWidth = (sizeof(CBufferPerFrame) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbPerFrameDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(&cbPerFrameDesc, nullptr, m_cbufferPerFrame.put()));

    // Create a linear sampler.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 4; // increase MaxAnisotropy if you use the D3D11_FILTER_ANISOTROPIC filter
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
    samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

    winrt::check_hresult(
        device->CreateSamplerState(
            &samplerDesc,
            m_linearSampler.put()));
}

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

    winrt::Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 0.25f * XM_PI;

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        500.0f);

    XMMATRIX orientationMatrix = m_deviceResources->GetOrientationTransform3D();

    projectionMatrix =
        XMMatrixTranspose(
            XMMatrixMultiply(
                orientationMatrix,
                projectionMatrix));

    CBufferOnResize cbufferOnResizeData;
    ZeroMemory(&cbufferOnResizeData, sizeof(cbufferOnResizeData));
    XMStoreFloat4x4(&cbufferOnResizeData.Projection, projectionMatrix);

    auto context{ m_deviceResources->GetD3DDeviceContext() };
    context->UpdateSubresource(m_cbufferOnResize.get(), 0, nullptr, &cbufferOnResizeData, 0, 0);
}

void CommonRenderer::Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix)
{
    if (!m_initialized)
        return;

    CBufferPerFrame cbufferPerFrameData;
    ZeroMemory(&cbufferPerFrameData, sizeof(cbufferPerFrameData));

    XMStoreFloat4x4(&cbufferPerFrameData.View, XMMatrixTranspose(viewMatrix));
    XMStoreFloat3(&cbufferPerFrameData.EyePosition, eye);

    auto context{ m_deviceResources->GetD3DDeviceContext() };
    context->UpdateSubresource(m_cbufferPerFrame.get(), 0, nullptr, &cbufferPerFrameData, 0, 0);
}

void CommonRenderer::PrepareRender()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Reset the viewport to target the whole screen.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Clear the views - the back buffer and the depth stencil view.
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();
    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (!m_initialized)
        return;

    // Bind the back buffer and the depth stencil view to the pipeline.
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Bind constant buffers.
    ID3D11Buffer* pCBufferNeverChanges{ m_cbufferNeverChanges.get() };
    ID3D11Buffer* pCBufferOnResize{ m_cbufferOnResize.get() };
    ID3D11Buffer* pCBufferPerFrame{ m_cbufferPerFrame.get() };
    context->PSSetConstantBuffers(0, 1, &pCBufferNeverChanges);
    context->VSSetConstantBuffers(1, 1, &pCBufferOnResize);
    context->VSSetConstantBuffers(2, 1, &pCBufferPerFrame);
    context->PSSetConstantBuffers(2, 1, &pCBufferPerFrame);

    // Set the sampler.
    ID3D11SamplerState* pLinearSampler{ m_linearSampler.get() };
    context->PSSetSamplers(0, 1, &pLinearSampler);
}

void CommonRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;

    m_cbufferNeverChanges = nullptr;
    m_cbufferOnResize = nullptr;
    m_cbufferPerFrame = nullptr;
    m_linearSampler = nullptr;
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

