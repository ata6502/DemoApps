#include "pch.h"

#include <DirectXColors.h>

#include "ConstantBuffers.h"
#include "FileReader.h"
#include "SkyRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

SkyRenderer::SkyRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_cbufferSky(nullptr),
    m_vertexShader(nullptr),
    m_pixelShader(nullptr),
    m_texture(nullptr),
    m_noCullRasterizerState(nullptr),
    m_lessEqualDepthStencilState(nullptr),
    m_initialized(false),
    m_texturePath(L"")
{
    m_skySphere = std::make_unique<SkySphere>(m_deviceResources);
}

SkyRenderer::~SkyRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction SkyRenderer::CreateDeviceResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Load shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"SkyVS.cso");
    auto pixelShaderBytecode = co_await Utilities::ReadDataAsync(L"SkyPS.cso");

    // Create vertex shader.
    winrt::check_hresult(
        device->CreateVertexShader(
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            nullptr,
            m_vertexShader.put()));

    // Create vertex description.
    static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // Create the input layout using the vertex description and the vertex shader bytecode.
    winrt::check_hresult(
        device->CreateInputLayout(
            vertexDesc,
            ARRAYSIZE(vertexDesc),
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            m_inputLayout.put()));

    // Create the pixel shader.
    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode.data(),
            pixelShaderBytecode.Length(),
            nullptr,
            m_pixelShader.put()));

    // Create constant buffer for the sky.
    uint32_t byteWidth = (sizeof(CBufferSky) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC CBufferSkyDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(device->CreateBuffer(&CBufferSkyDesc, nullptr, m_cbufferSky.put()));

    // Create the no culling rasterizer state.
    D3D11_RASTERIZER_DESC noCullDesc;
    ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
    noCullDesc.FillMode = D3D11_FILL_SOLID;
    noCullDesc.CullMode = D3D11_CULL_NONE;
    noCullDesc.FrontCounterClockwise = false;
    noCullDesc.DepthClipEnable = true;

    winrt::check_hresult(
        device->CreateRasterizerState(
            &noCullDesc,
            m_noCullRasterizerState.put()));

    // Set the depth function to LESS_EQUAL and not just LESS. Otherwise, the normalized 
    // depth values at z = 1 (NDC) will fail the depth test if the depth buffer is cleared to 1.
    D3D11_DEPTH_STENCIL_DESC lessEqualDepthFuncDesc;
    lessEqualDepthFuncDesc.DepthEnable = true;
    lessEqualDepthFuncDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    lessEqualDepthFuncDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    lessEqualDepthFuncDesc.StencilEnable = false; // do not use stencil
    // We are not using stencil, so the stencil settings do not matter.
    lessEqualDepthFuncDesc.StencilReadMask = 0xff;
    lessEqualDepthFuncDesc.StencilWriteMask = 0xff;
    lessEqualDepthFuncDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    lessEqualDepthFuncDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    lessEqualDepthFuncDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    lessEqualDepthFuncDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    lessEqualDepthFuncDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    lessEqualDepthFuncDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    lessEqualDepthFuncDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    lessEqualDepthFuncDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    winrt::check_hresult(
        device->CreateDepthStencilState(
            &lessEqualDepthFuncDesc, 
            m_lessEqualDepthStencilState.put()));

    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void SkyRenderer::Update(DirectX::FXMVECTOR eye)
{
    if (!m_initialized)
        return;

    XMFLOAT3 eyePosition;
    XMStoreFloat3(&eyePosition, eye);

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferSky cbufferSkyData;
    ZeroMemory(&cbufferSkyData, sizeof(cbufferSkyData));

    // Center the sky about eye in world space.
    XMStoreFloat4x4(&cbufferSkyData.WorldSky,
        XMMatrixTranspose(
            XMMatrixTranslation(
                eyePosition.x, eyePosition.y, eyePosition.z)));

    context->UpdateSubresource(m_cbufferSky.get(), 0, nullptr, &cbufferSkyData, 0, 0);
}

void SkyRenderer::Render()
{
    if (!m_initialized)
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Bind the constant buffers.
    ID3D11Buffer* pCBufferSky{ m_cbufferSky.get() };
    context->VSSetConstantBuffers(3, 1, &pCBufferSky);
    context->PSSetConstantBuffers(3, 1, &pCBufferSky);

    context->IASetInputLayout(m_inputLayout.get());
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.get(), nullptr, 0);
    m_skySphere->SetBuffers();

    // Set the cube map texture.
    ID3D11ShaderResourceView* pTexture{ m_texture.get() };
    context->PSSetShaderResources(0, 1, &pTexture);

    // Disable culling of backfaces.
    context->RSSetState(m_noCullRasterizerState.get());

    // Set the depth function to LESS_EQUAL. 
    // StencilRef does not matter because this state does not use stencil.
    context->OMSetDepthStencilState(m_lessEqualDepthStencilState.get(), 1); 

    // Draw the mesh.
    m_skySphere->DrawSphere();

    // Cleanup.
    context->RSSetState(nullptr);
    context->OMSetDepthStencilState(nullptr, 0);
    context->IASetInputLayout(nullptr);
    context->VSSetShader(nullptr, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);
    context->VSSetConstantBuffers(0, 0, nullptr);
    context->PSSetConstantBuffers(0, 0, nullptr);
}

void SkyRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;
    m_skySphere->ReleaseDeviceDependentResources();

    m_cbufferSky = nullptr;
    m_inputLayout = nullptr;
    m_vertexShader = nullptr;
    m_pixelShader = nullptr;
    m_texture = nullptr;
    m_noCullRasterizerState = nullptr;
    m_lessEqualDepthStencilState = nullptr;
}

void SkyRenderer::CreateSkySphereMesh(float radius, uint32_t sliceCount, uint32_t stackCount)
{
    m_skySphere->CreateSphere(radius, sliceCount, stackCount);
}

winrt::Windows::Foundation::IAsyncAction SkyRenderer::LoadTexture(std::wstring const& path)
{
    auto device{ m_deviceResources->GetD3DDevice() };

    co_await FileReader::LoadTextureAsync(device, path.c_str(), m_texture.put());
}
