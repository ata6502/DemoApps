#include "pch.h"

#include "FileReader.h"
#include "SceneConstantBuffers.h"
#include "SceneRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

SceneRenderer::SceneRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<TextureMeshGenerator> const& meshGenerator) :
    m_deviceResources(deviceResources),
    m_meshGenerator(meshGenerator),
    m_inputLayout(nullptr),
    m_vertexShader(nullptr),
    m_pixelShader(nullptr),
    m_linearSampler(nullptr),
    m_cbufferPerFrame(nullptr),
    m_cbufferPerObject(nullptr),
    m_comparisonSampler(nullptr),
    m_initialized(false),
    m_rotation(0.f),
    m_elapsedSeconds(0.f),
    m_lightRotationAngle(0.0f)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());
    ZeroMemory(&m_directionalLight, sizeof(m_directionalLight));
    ZeroMemory(&m_originalLightDirection, sizeof(m_originalLightDirection));
    ZeroMemory(&m_shadowTransform, sizeof(m_shadowTransform));

    CreateMaterials();
    CreateTextureTransforms();
}

SceneRenderer::~SceneRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction SceneRenderer::CreateDeviceDependentResourcesAsync()
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

    // Create the linear sampler state.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 4;
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

    // Create constant buffers and ensure that the buffer size is a multiple of 16.
    uint32_t alignedBufferSize = (sizeof(CBufferPerFrame) + 15) & ~15;
    CD3D11_BUFFER_DESC cbufferPerFrameDesc(alignedBufferSize, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(device->CreateBuffer(&cbufferPerFrameDesc, nullptr, m_cbufferPerFrame.put()));

    alignedBufferSize = (sizeof(CBufferPerObject) + 15) & ~15;
    CD3D11_BUFFER_DESC constantBufferPerObjectDesc(alignedBufferSize, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(device->CreateBuffer(&constantBufferPerObjectDesc, nullptr, m_cbufferPerObject.put()));

    // Load textures.
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\bricks.dds", m_textures["bricks"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\marble.dds", m_textures["marble"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\floor.dds", m_textures["floor"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\wood.dds", m_textures["wood"].put());

    // Create the comparison sampler state. 
    D3D11_SAMPLER_DESC comparisonSamplerDesc;
    ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // for PCF, use the comparison filter
    comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.MipLODBias = 0;
    comparisonSamplerDesc.MaxAnisotropy = 1; 
    comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; // for PCF, the value to compare against the shadow map samples
    comparisonSamplerDesc.BorderColor[0] = 0.0f; // Red
    comparisonSamplerDesc.BorderColor[1] = 0.0f; // Green
    comparisonSamplerDesc.BorderColor[2] = 0.0f; // Blue
    comparisonSamplerDesc.BorderColor[3] = 0.0f;
    comparisonSamplerDesc.MinLOD = 0; 
    comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    winrt::check_hresult(
        device->CreateSamplerState(
            &comparisonSamplerDesc,
            m_comparisonSampler.put()));
}

// Create context-dependent resources.
void SceneRenderer::FinalizeCreateDeviceResources()
{
    // Create the directional light.
    m_directionalLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_directionalLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_directionalLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_directionalLight.Direction = XMFLOAT3(0.51451f, -0.51451f, 0.68601f);

    m_originalLightDirection = m_directionalLight.Direction;

    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void SceneRenderer::SetProjectionMatrix(DirectX::FXMMATRIX projectionMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projectionMatrix);
}

void SceneRenderer::Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, DirectX::XMFLOAT4X4 shadowTransform, float rotation, float elapsedSeconds)
{
    m_rotation = rotation;
    m_elapsedSeconds = elapsedSeconds;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferPerFrame cbufferPerFrameData;
    ZeroMemory(&cbufferPerFrameData, sizeof(cbufferPerFrameData));

    XMStoreFloat4x4(&cbufferPerFrameData.View, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cbufferPerFrameData.Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_projMatrix)));
    XMStoreFloat3(&cbufferPerFrameData.EyePosition, eyePosition);
    cbufferPerFrameData.DirectionalLight = m_directionalLight;

    context->UpdateSubresource(m_cbufferPerFrame.get(), 0, nullptr, &cbufferPerFrameData, 0, 0);

    m_shadowTransform = shadowTransform;
}

DirectX::XMVECTOR SceneRenderer::UpdateLightDirection()
{
    // Animate the light.
    m_lightRotationAngle += 0.1f * m_elapsedSeconds;
    XMMATRIX R = XMMatrixRotationY(m_lightRotationAngle);
    XMVECTOR lightDir = XMLoadFloat3(&m_originalLightDirection);
    lightDir = XMVector3TransformNormal(lightDir, R);
    XMStoreFloat3(&m_directionalLight.Direction, lightDir);

    return lightDir;
}

void SceneRenderer::Render(ID3D11ShaderResourceView* pShadowMapTexture)
{
    if (!IsInitialized())
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Bind the shaders.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.get(), nullptr, 0);

    // Set the linear sampler.
    ID3D11SamplerState* pLinearSampler{ m_linearSampler.get() };
    context->PSSetSamplers(0, 1, &pLinearSampler);

    // Bind the cbuffers
    ID3D11Buffer* pCBufferPerFrame{ m_cbufferPerFrame.get() };
    ID3D11Buffer* pCBufferPerObject{ m_cbufferPerObject.get() };
    context->VSSetConstantBuffers(0, 1, &pCBufferPerFrame);
    context->PSSetConstantBuffers(0, 1, &pCBufferPerFrame);
    context->VSSetConstantBuffers(1, 1, &pCBufferPerObject);
    context->PSSetConstantBuffers(1, 1, &pCBufferPerObject);

    // Set the comparison sampler for PCF filtering. It is used in the ScenePS shader.
    ID3D11SamplerState* pComparisonSampler{ m_comparisonSampler.get() };
    context->PSSetSamplers(1, 1, &pComparisonSampler);

    // Set the shadow map texture (a.k.a. depth map).
    context->PSSetShaderResources(1, 1, &pShadowMapTexture);

    // Draw the scene.
    DrawScene();

    // Unbind the cbuffers.
    context->VSSetConstantBuffers(0, 0, nullptr);
    context->PSSetConstantBuffers(0, 0, nullptr);
    context->VSSetConstantBuffers(1, 0, nullptr);
    context->PSSetConstantBuffers(1, 0, nullptr);

    // Unbind the shaders.
    context->VSSetShader(nullptr, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);

    // Unbind the samplers.
    ID3D11SamplerState* pNullSampler{ nullptr };
    context->PSSetSamplers(0, 1, &pNullSampler);
    context->PSSetSamplers(1, 1, &pNullSampler);

    // Unbind the textures (ShaderResources).
    ID3D11ShaderResourceView* pNullTexture{ nullptr };
    context->PSSetShaderResources(0, 1, &pNullTexture);
    context->PSSetShaderResources(1, 1, &pNullTexture);
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;

    m_inputLayout = nullptr;
    m_vertexShader = nullptr;
    m_pixelShader = nullptr;
    m_linearSampler = nullptr;
    m_cbufferPerFrame = nullptr;
    m_cbufferPerObject = nullptr;
    m_comparisonSampler = nullptr;

    m_textures.clear();
}

void SceneRenderer::CreateMaterials()
{
    MaterialDesc material;
    material.Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    material.Diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 8.0f); // w = SpecularPower
    m_materials["floor"] = material;

    material.Ambient = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
    material.Diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
    material.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 8.0f); // w = SpecularPower
    m_materials["column"] = material;

    material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f); // w = SpecularPower
    m_materials["ball"] = material;

    material.Ambient = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
    material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 8.0f); // w = SpecularPower
    m_materials["box"] = material;
}

void SceneRenderer::CreateTextureTransforms()
{
    XMStoreFloat4x4(
        &m_floorTextureTransform,
        XMMatrixScaling(6.f, 6.f, 0.f));

    XMStoreFloat4x4(
        &m_columnTextureTransform,
        XMMatrixScaling(1.f, 3.f, 0.f));
}

void SceneRenderer::DrawScene()
{
    // Draw the floor.
    DrawObject(XMMatrixIdentity(), XMLoadFloat4x4(&m_floorTextureTransform), "floor", "floor", "grid");

    // Draw a spinning cube in the middle of the floor.
    DrawObject(
        XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixRotationX(XM_PIDIV4) * XMMatrixRotationZ(XM_PIDIV4) * XMMatrixRotationY(m_rotation) * XMMatrixTranslation(0.0f, 4.0f, 0.0f),
        XMMatrixIdentity(), "box", "wood", "cube");

    // Draw columns.
    DrawObject(XMMatrixTranslation(-4.0f, 2.5f, -4.0f), XMLoadFloat4x4(&m_columnTextureTransform), "column", "bricks", "cylinder");
    DrawObject(XMMatrixTranslation(4.0f, 2.5f, -4.0f), XMLoadFloat4x4(&m_columnTextureTransform), "column", "bricks", "cylinder");
    DrawObject(XMMatrixTranslation(-4.0f, 2.5f, 4.0f), XMLoadFloat4x4(&m_columnTextureTransform), "column", "bricks", "cylinder");
    DrawObject(XMMatrixTranslation(4.0f, 2.5f, 4.0f), XMLoadFloat4x4(&m_columnTextureTransform), "column", "bricks", "cylinder");

    // Draw balls.
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-4.0f, 5.5f, -4.0f), XMMatrixIdentity(), "ball", "marble", "sphere");
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(4.0f, 5.5f, -4.0f), XMMatrixIdentity(), "ball", "marble", "sphere");
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-4.0f, 5.5f, 4.0f), XMMatrixIdentity(), "ball", "marble", "sphere");
    DrawObject(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(4.0f, 5.5f, 4.0f), XMMatrixIdentity(), "ball", "marble", "sphere");
}

void SceneRenderer::DrawObject(DirectX::FXMMATRIX worldMatrix, DirectX::CXMMATRIX textureTransform, std::string materialName, std::string textureName, std::string meshName)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    CBufferPerObject cbufferPerObjectData;
    ZeroMemory(&cbufferPerObjectData, sizeof(cbufferPerObjectData));

    // Update the constant buffer.
    XMStoreFloat4x4(&cbufferPerObjectData.World,
        XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cbufferPerObjectData.WorldInvTranspose,
        XMMatrixTranspose(Utilities::CalculateInverseTranspose(worldMatrix)));
    XMStoreFloat4x4(&cbufferPerObjectData.TextureTransform,
        XMMatrixTranspose(textureTransform));
    cbufferPerObjectData.Material = 
        m_materials[materialName];
    XMStoreFloat4x4(&cbufferPerObjectData.ShadowTransform,
        XMMatrixTranspose(XMMatrixMultiply(worldMatrix, XMLoadFloat4x4(&m_shadowTransform))));

    context->UpdateSubresource(m_cbufferPerObject.get(), 0, nullptr, &cbufferPerObjectData, 0, 0);

    // Set the texture.
    ID3D11ShaderResourceView* pTexture{ m_textures[textureName].get() };
    context->PSSetShaderResources(0, 1, &pTexture);

    // Draw the mesh.
    m_meshGenerator->DrawMesh(meshName);
}

