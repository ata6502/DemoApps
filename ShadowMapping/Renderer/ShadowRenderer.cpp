#include "pch.h"

#include "FileReader.h"
#include "ShadowConstantBuffers.h"
#include "ShadowRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

ShadowRenderer::ShadowRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<TextureMeshGenerator> const& meshGenerator) :
    m_deviceResources(deviceResources),
    m_meshGenerator(meshGenerator),
    m_inputLayout(nullptr),
    m_vertexShader(nullptr),
    m_cbufferPerFrame(nullptr),
    m_cbufferPerObject(nullptr),
    m_initialized(false),
    m_rotation(0.0f),
    m_rasterStateDepthBias(nullptr)
{
    ZeroMemory(&m_viewMatrix, sizeof(m_viewMatrix));
    ZeroMemory(&m_projMatrix, sizeof(m_projMatrix));
    ZeroMemory(&m_shadowTransform, sizeof(m_shadowTransform));

    m_shadowMap = std::make_unique<ShadowMap>(ShadowMapWidth, ShadowMapHeight);

    // Create the scene bounds.
    m_sceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_sceneBounds.Radius = sqrtf(10.0f * 10.0f + 12.5f * 12.5f);
}

ShadowRenderer::~ShadowRenderer()
{
    ReleaseDeviceDependentResources();
}

winrt::Windows::Foundation::IAsyncAction ShadowRenderer::CreateDeviceDependentResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    m_shadowMap->CreateDeviceDependentResources(device);

    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"ShadowVS.cso");

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

    // Create constant buffers.
    uint32_t byteWidth = (sizeof(CBufferPerFrame) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC cbufferPerFrameForShadowsDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(device->CreateBuffer(&cbufferPerFrameForShadowsDesc, nullptr, m_cbufferPerFrame.put()));

    byteWidth = (sizeof(CBufferPerObject) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC constantBufferPerObjectDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(device->CreateBuffer(&constantBufferPerObjectDesc, nullptr, m_cbufferPerObject.put()));

    // Create a rasterizer state with depth bias settings.
    D3D11_RASTERIZER_DESC2 rasterStateDepthBiasDesc;
    ZeroMemory(&rasterStateDepthBiasDesc, sizeof(D3D11_RASTERIZER_DESC2));
    rasterStateDepthBiasDesc.AntialiasedLineEnable = false;
    rasterStateDepthBiasDesc.CullMode = D3D11_CULL_BACK;
    rasterStateDepthBiasDesc.DepthClipEnable = true;
    rasterStateDepthBiasDesc.FillMode = D3D11_FILL_SOLID;
    rasterStateDepthBiasDesc.FrontCounterClockwise = true;
    rasterStateDepthBiasDesc.MultisampleEnable = false;
    rasterStateDepthBiasDesc.ScissorEnable = false;

    // Properties related to depth bias. 
    rasterStateDepthBiasDesc.DepthBias = 5000;             // a fixed bias; default 0
    rasterStateDepthBiasDesc.DepthBiasClamp = 0.0f;        // a maximum depth bias allowed; for very steep slopes, the bias may be too high and cause peter-panning; default 0.0f
    rasterStateDepthBiasDesc.SlopeScaledDepthBias = 1.0f;  // a scale factor to control how much to bias based on the polygon slope; default 0.0f

    winrt::check_hresult(
        device->CreateRasterizerState2(
            &rasterStateDepthBiasDesc,
            m_rasterStateDepthBias.put()));
}

// Create context-dependent resources.
void ShadowRenderer::FinalizeCreateDeviceResources()
{
    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void ShadowRenderer::Update(float rotation)
{
    m_rotation = rotation;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferPerFrame cbufferPerFrameData;
    ZeroMemory(&cbufferPerFrameData, sizeof(cbufferPerFrameData));

    XMStoreFloat4x4(&cbufferPerFrameData.View, XMMatrixTranspose(XMLoadFloat4x4(&m_viewMatrix)));
    XMStoreFloat4x4(&cbufferPerFrameData.Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_projMatrix)));

    context->UpdateSubresource(m_cbufferPerFrame.get(), 0, nullptr, &cbufferPerFrameData, 0, 0);
}

void ShadowRenderer::Render()
{
    if (!IsInitialized())
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Bind the shaders.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);

    // Bind the cbuffer.
    ID3D11Buffer* pCBufferPerFrame{ m_cbufferPerFrame.get() };
    ID3D11Buffer* pCBufferPerObject{ m_cbufferPerObject.get() };
    context->VSSetConstantBuffers(0, 1, &pCBufferPerFrame);
    context->VSSetConstantBuffers(1, 1, &pCBufferPerObject);

    // Prepare the OM stage for rendering to the shadow map.
    m_shadowMap->BindResources(context);

    // Apply the bias when we render the scene to the shadow map.
    context->RSSetState(m_rasterStateDepthBias.get());

    // Render the scene into the shadow map.
    DrawSceneToShadowMap();

    // Unbind the cbuffers.
    context->VSSetConstantBuffers(0, 0, nullptr); 
    context->VSSetConstantBuffers(1, 0, nullptr);

    // Unbind the shader.
    context->VSSetShader(nullptr, nullptr, 0);

    // Reset the rasterizer state.
    context->RSSetState(nullptr);
}

void ShadowRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;

    m_shadowMap->ReleaseDeviceDependentResources();
    m_inputLayout = nullptr;
    m_vertexShader = nullptr;
    m_cbufferPerFrame = nullptr;
    m_cbufferPerObject = nullptr;
    m_rasterStateDepthBias = nullptr;
}

// The following code is based on [Luna]
DirectX::XMFLOAT4X4 ShadowRenderer::BuildShadowTransform(DirectX::FXMVECTOR lightDirection)
{
    // Derive the light view matrix from the light source.
    XMVECTOR lightPos = -2.0f * m_sceneBounds.Radius * lightDirection;
    XMVECTOR targetPos = XMLoadFloat3(&m_sceneBounds.Center);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

    // Transform the bounding sphere to light space.
    XMFLOAT3 sphereCenterLS;
    XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

    // Compute the light view volume to fit the bounding sphere of the entire scene.
    float l = sphereCenterLS.x - m_sceneBounds.Radius;
    float b = sphereCenterLS.y - m_sceneBounds.Radius;
    float n = sphereCenterLS.z - m_sceneBounds.Radius;
    float r = sphereCenterLS.x + m_sceneBounds.Radius;
    float t = sphereCenterLS.y + m_sceneBounds.Radius;
    float f = sphereCenterLS.z + m_sceneBounds.Radius;

    // Define the light projection matrix representing the light view volume. 
    XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    // Build the shadow transform.
    XMMATRIX S = V * P * T;

    // Store the light transforms.
    XMStoreFloat4x4(&m_viewMatrix, V);
    XMStoreFloat4x4(&m_projMatrix, P);
    XMStoreFloat4x4(&m_shadowTransform, S);

    return m_shadowTransform;
}

void ShadowRenderer::DrawSceneToShadowMap()
{
    // Draw a spinning cube in the middle of the floor.
    XMMATRIX cubeTransform = XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixRotationX(XM_PIDIV4) * XMMatrixRotationZ(XM_PIDIV4) * XMMatrixRotationY(m_rotation) * XMMatrixTranslation(0.0f, 4.0f, 0.0f);
    DrawObject(cubeTransform, "cube");

    // Draw columns.
    DrawObject(XMMatrixTranslation(-4.0f, 2.5f, -4.0f), "cylinder");
    DrawObject(XMMatrixTranslation(4.0f, 2.5f, -4.0f), "cylinder");
    DrawObject(XMMatrixTranslation(-4.0f, 2.5f, 4.0f), "cylinder");
    DrawObject(XMMatrixTranslation(4.0f, 2.5f, 4.0f), "cylinder");

    // Draw balls.
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-4.0f, 5.5f, -4.0f), "sphere");
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(4.0f, 5.5f, -4.0f), "sphere");
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-4.0f, 5.5f, 4.0f), "sphere");
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(4.0f, 5.5f, 4.0f), "sphere");
}

void ShadowRenderer::DrawObject(DirectX::FXMMATRIX worldMatrix, std::string meshName)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferPerObject cbufferPerObjectData;
    ZeroMemory(&cbufferPerObjectData, sizeof(cbufferPerObjectData));

    // Update the constant buffer.
    XMStoreFloat4x4(&cbufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    context->UpdateSubresource(m_cbufferPerObject.get(), 0, nullptr, &cbufferPerObjectData, 0, 0);

    // Draw the mesh.
    m_meshGenerator->DrawMesh(meshName);
}

