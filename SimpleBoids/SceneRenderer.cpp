#include "pch.h"

#include <DirectXColors.h>

#include "FileReader.h"
#include "SceneRenderer.h"
#include "Utilities.h"

using namespace DirectX;

SceneRenderer::SceneRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false),
    m_vertexShader(nullptr),
    m_inputLayout(nullptr),
    m_pixelShader(nullptr),
    m_cbufferPerObject(nullptr),
    m_cbufferPerObjectData(),
    m_transparentBlendState(nullptr)
{
    m_meshGenerator = std::make_unique<TextureMeshGenerator>(m_deviceResources);
}

SceneRenderer::~SceneRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction SceneRenderer::CreateDeviceResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Load shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"SceneVS.cso");
    auto pixelShaderBytecode = co_await Utilities::ReadDataAsync(L"ScenePS.cso");

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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

    // Create constant buffers.
    uint32_t byteWidth = (sizeof(CBufferPerObject) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbPerObjectDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(&cbPerObjectDesc, nullptr, m_cbufferPerObject.put()));

    // Create the transparent blend state.
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    winrt::check_hresult(
        device->CreateBlendState(
            &blendDesc,
            m_transparentBlendState.put()));
}

void SceneRenderer::FinalizeCreateDeviceResources()
{
    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

// TODO: Remove CreateWindowSizeDependentResources if not needed.
void SceneRenderer::CreateWindowSizeDependentResources()
{
    if (!m_initialized)
        return;
}

// TODO: Remove Update if not needed.
void SceneRenderer::Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix)
{
    if (!m_initialized)
        return;
}

void SceneRenderer::PrepareRender()
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

    // Bind the back buffer and the depth stencil view to the pipeline.
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    if (!m_initialized)
        return;

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Attach shaders.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.get(), nullptr, 0);

    m_meshGenerator->SetBuffers();

    // Bind constant buffers.
    ID3D11Buffer* pCBufferPerObject{ m_cbufferPerObject.get() };
    context->VSSetConstantBuffers(3, 1, &pCBufferPerObject);
    context->PSSetConstantBuffers(3, 1, &pCBufferPerObject);

    ZeroMemory(&m_cbufferPerObjectData, sizeof(m_cbufferPerObjectData));
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;
    m_meshGenerator->Clear();
    m_textures.clear();

    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_cbufferPerObject = nullptr;
    m_transparentBlendState = nullptr;
}

void SceneRenderer::CreateSphereMesh(std::string const& name, float radius, uint16_t subdivisionCount)
{
    m_meshGenerator->CreateGeosphere(name, radius, subdivisionCount);
}

void SceneRenderer::CreateCylinderMesh(std::string const& name, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
{
    m_meshGenerator->CreateCylinder(name, bottomRadius, topRadius, cylinderHeight, sliceCount, stackCount);
}

void SceneRenderer::CreateCubeMesh(std::string const& name)
{
    m_meshGenerator->CreateCube(name);
}

void SceneRenderer::CreateGridMesh(std::string const& name, float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth)
{
    m_meshGenerator->CreateGrid(name, gridWidth, gridDepth, quadCountHoriz, quadCountDepth);
}

void SceneRenderer::FinalizeCreateMeshes()
{
    m_meshGenerator->CreateBuffers();
}

void SceneRenderer::RenderMesh(std::string const& name)
{
    if (!m_initialized)
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };
    context->UpdateSubresource(m_cbufferPerObject.get(), 0, nullptr, &m_cbufferPerObjectData, 0, 0);
    m_meshGenerator->DrawMesh(name);
}

void SceneRenderer::SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
{
    // Calculate the world inverse transpose matrix in order to properly transform normals in case there are any non-uniform or shear transformations.
    auto worldInvTranspose = Utilities::CalculateInverseTranspose(worldMatrix);
    XMStoreFloat4x4(&m_cbufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&m_cbufferPerObjectData.WorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
}

void SceneRenderer::AddMaterial(std::string const& name, MaterialDesc const& material)
{
    m_materials[name] = material;
}

void SceneRenderer::SetMaterial(std::string const& name)
{
    m_cbufferPerObjectData.Material = m_materials[name];
}

winrt::Windows::Foundation::IAsyncAction SceneRenderer::AddTexture(std::string const& name, std::wstring const& path)
{
    auto device{ m_deviceResources->GetD3DDevice() };

    co_await FileReader::LoadTextureAsync(device, path.c_str(), m_textures[name].put());
}

void SceneRenderer::SetTexture(std::string const& name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    ID3D11ShaderResourceView* pTexture{ m_textures[name].get() };
    context->PSSetShaderResources(0, 1, &pTexture);
}

void SceneRenderer::SetTextureTransform(DirectX::FXMMATRIX textureTransform)
{
    XMStoreFloat4x4(&m_cbufferPerObjectData.TextureTransform, XMMatrixTranspose(textureTransform)); 
}

// Binds the transparent blend state object to the output merger stage.
void SceneRenderer::SetTransparentBlendState()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // An array of four floats defining an RGBA color vector used as a blend factor when D3D11_BLEND_BLEND_FACTOR or D3D11_BLEND_INV_BLEND_FACTOR is specified.
    float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };

    context->OMSetBlendState(
        m_transparentBlendState.get(),  // a pointer to a blend state to enable
        blendFactor,                    
        0xffffffff);
}

void SceneRenderer::ClearTransparentBlendState()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };

    // Set the default blend state.
    UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(nullptr, blendFactor, sampleMask);
}
