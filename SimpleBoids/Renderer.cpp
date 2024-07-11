#include "pch.h"

#include <DirectXColors.h>

#include "FileReader.h"
#include "Renderer.h"
#include "Utilities.h"

using namespace DirectX;

Renderer::Renderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false),
    m_vertexShader(nullptr),
    m_inputLayout(nullptr),
    m_pixelShader(nullptr),
    m_cbufferNeverChanges(nullptr),
    m_cbufferOnResize(nullptr),
    m_cbufferPerFrame(nullptr),
    m_cbufferPerObject(nullptr),
    m_linearSampler(nullptr),
    m_cbufferPerObjectData()
{
    m_meshGenerator = std::make_unique<TextureMeshGenerator>(m_deviceResources);
}

Renderer::~Renderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction Renderer::CreateDeviceResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Load shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"MaterialVS.cso");
    auto pixelShaderBytecode = co_await Utilities::ReadDataAsync(L"MaterialPS.cso");

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

    byteWidth = (sizeof(CBufferPerObject) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbPerObjectDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(&cbPerObjectDesc, nullptr, m_cbufferPerObject.put()));

    // Create a linear sampler state.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 4; // increase MaxAnisotropy if you use the D3D11_FILTER_ANISOTROPIC filter
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0.0f; // Red
    samplerDesc.BorderColor[1] = 1.0f; // Green
    samplerDesc.BorderColor[2] = 0.0f; // Blue
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
    samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

    winrt::check_hresult(
        device->CreateSamplerState(
            &samplerDesc,
            m_linearSampler.put()));
}

// Create context-dependent resources.
void Renderer::FinalizeCreateDeviceResources()
{
    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void Renderer::CreateWindowSizeDependentResources()
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

void Renderer::Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix)
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

void Renderer::PrepareRender()
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
    ID3D11Buffer* pCBufferNeverChanges{ m_cbufferNeverChanges.get() };
    ID3D11Buffer* pCBufferOnResize{ m_cbufferOnResize.get() };
    ID3D11Buffer* pCBufferPerFrame{ m_cbufferPerFrame.get() };
    ID3D11Buffer* pCBufferPerObject{ m_cbufferPerObject.get() };
    context->PSSetConstantBuffers(0, 1, &pCBufferNeverChanges);
    context->VSSetConstantBuffers(1, 1, &pCBufferOnResize);
    context->VSSetConstantBuffers(2, 1, &pCBufferPerFrame);
    context->PSSetConstantBuffers(2, 1, &pCBufferPerFrame);
    context->VSSetConstantBuffers(3, 1, &pCBufferPerObject);
    context->PSSetConstantBuffers(3, 1, &pCBufferPerObject);

    // Set the sampler.
    ID3D11SamplerState* pLinearSampler{ m_linearSampler.get() };
    context->PSSetSamplers(0, 1, &pLinearSampler);

    ZeroMemory(&m_cbufferPerObjectData, sizeof(m_cbufferPerObjectData));
}

void Renderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;
    m_meshGenerator->Clear();
    m_textures.clear();

    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_cbufferNeverChanges = nullptr;
    m_cbufferOnResize = nullptr;
    m_cbufferPerFrame = nullptr;
    m_cbufferPerObject = nullptr;
    m_linearSampler = nullptr;
}

void Renderer::CreateSphereMesh(std::string const& name, float radius, uint16_t subdivisionCount)
{
    m_meshGenerator->CreateGeosphere(name, radius, subdivisionCount);
}

void Renderer::CreateConeMesh(std::string const& name)
{
    m_meshGenerator->CreateCylinder(name, 2.f, 0.f, 5.f, 12, 4);
}

void Renderer::CreateCubeMesh(std::string const& name)
{
    m_meshGenerator->CreateCube(name);
}

void Renderer::FinalizeCreateMeshes()
{
    m_meshGenerator->CreateBuffers();
}

void Renderer::RenderMesh(std::string const& name)
{
    if (!m_initialized)
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };
    context->UpdateSubresource(m_cbufferPerObject.get(), 0, nullptr, &m_cbufferPerObjectData, 0, 0);
    m_meshGenerator->DrawMesh(name);
}

void Renderer::SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
{
    // Calculate the world inverse transpose matrix in order to properly transform normals in case there are any non-uniform or shear transformations.
    auto worldInvTranspose = Utilities::CalculateInverseTranspose(worldMatrix);
    XMStoreFloat4x4(&m_cbufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&m_cbufferPerObjectData.WorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
}

void Renderer::SetLight(DirectionalLightDesc light)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferNeverChanges cbufferNeverChangesData;
    ZeroMemory(&cbufferNeverChangesData, sizeof(cbufferNeverChangesData));
    cbufferNeverChangesData.Light = light;

    // Copy the light to the constant buffer.
    context->UpdateSubresource(m_cbufferNeverChanges.get(), 0, nullptr, &cbufferNeverChangesData, 0, 0);
}

void Renderer::AddMaterial(std::string const& name, MaterialDesc const& material)
{
    m_materials[name] = material;
}

void Renderer::SetMaterial(std::string const& name)
{
    m_cbufferPerObjectData.Material = m_materials[name];
}

winrt::Windows::Foundation::IAsyncAction Renderer::AddTexture(std::string const& name, std::wstring const& path)
{
    auto device{ m_deviceResources->GetD3DDevice() };

    co_await FileReader::LoadTextureAsync(device, path.c_str(), m_textures[name].put());
}

void Renderer::SetTexture(std::string const& name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    ID3D11ShaderResourceView* pTexture{ m_textures[name].get() };
    context->PSSetShaderResources(0, 1, &pTexture);
}

void Renderer::SetTextureTransform(DirectX::FXMMATRIX textureTransform)
{
    XMStoreFloat4x4(&m_cbufferPerObjectData.TextureTransform, XMMatrixTranspose(textureTransform)); 
}