#include "pch.h"

#include "BlendingRenderer.h"
#include "FileReader.h"
#include "LightsShaderStructures.h"
#include "MathHelper.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

BlendingRenderer::BlendingRenderer(
    std::shared_ptr<DX::DeviceResources> const& deviceResources, 
    std::shared_ptr<MaterialController> const& materialController,
    std::shared_ptr<LightsController> const& lightsController,
    std::shared_ptr<FogController> const& fogController) :
    m_deviceResources(deviceResources),
    m_materialController(materialController),
    m_lightsController(lightsController),
    m_fogController(fogController),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_terrainMesh = std::make_unique<GridMesh>(deviceResources);
    m_boxMesh = std::make_unique<BoxMesh>(deviceResources);
    m_stateManager = std::make_unique<StateManager>(m_deviceResources);

    XMStoreFloat4x4(&m_grassTextureTransform, XMMatrixIdentity());
    XMStoreFloat4x4(&m_waterTextureTransform, XMMatrixIdentity());

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::fire_and_forget BlendingRenderer::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"TextureVertexShader.cso");
    auto pixelShaderBytecode = co_await Utilities::ReadDataAsync(L"BlendingPixelShader.cso");

    // [2] Create vertex shader.
    winrt::check_hresult(
        device->CreateVertexShader(
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            nullptr,
            m_vertexShader.put()));

    // [3] Create vertex description.
    static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // [4] Create the input layout using the vertex description and the vertex shader bytecode.
    winrt::check_hresult(
        device->CreateInputLayout(
            vertexDesc,
            ARRAYSIZE(vertexDesc),
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            m_inputLayout.put()));

    // [5] Create the pixel shader.
    winrt::check_hresult(
        device->CreatePixelShader(
            pixelShaderBytecode.data(),
            pixelShaderBytecode.Length(),
            nullptr,
            m_pixelShader.put()));

    // [6] Create constant buffers.
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    bd.ByteWidth = (sizeof(ConstantBufferPerFrame) + 15) / 16 * 16;
    winrt::check_hresult(
        device->CreateBuffer(&bd, nullptr, m_constantBufferPerFrame.put()));

    bd.ByteWidth = (sizeof(ConstantBufferPerObject) + 15) / 16 * 16;
    winrt::check_hresult(
        device->CreateBuffer(&bd, nullptr, m_constantBufferPerObject.put()));

    // Initialize wave data.
    m_waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

    // [7] Create the vertex buffer. Note that we only allocate space. 
    // We will be updating the data every frame.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexPositionTexture) * m_waves.VertexCount();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;

    winrt::check_hresult(
        device->CreateBuffer(
            &vertexBufferDesc,
            0, // no inital data
            m_waveVertexBuffer.put()));

    // [8] Create the index buffer. The index buffer is fixed, so we only need to create it once.
    std::vector<uint32_t> indices(3 * m_waves.TriangleCount()); // 3 indices per face

    // Iterate over each quad.
    uint32_t m = m_waves.RowCount();
    uint32_t n = m_waves.ColumnCount();
    int k = 0;
    for (uint32_t i = 0; i < m - 1; ++i)
    {
        for (uint32_t j = 0; j < n - 1; ++j)
        {
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i + 1) * n + j;

            indices[k + 3] = (i + 1) * n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = (i + 1) * n + j + 1;

            k += 6; // next quad
        }
    }

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(sizeof(uint32_t) * indices.size(), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_waveIndexBuffer.put()));

    // [9] Load textures from files.
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\grass.dds", m_textures["grass"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\water.dds", m_textures["water"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\wire.dds", m_textures["box"].put());

    // [10] Create a sampler state.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // use linear filtering for minification, magnification, and mipmapping
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 1.0f; // Red
    samplerDesc.BorderColor[1] = 1.0f; // Green
    samplerDesc.BorderColor[2] = 1.0f; // Blue
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
    samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

    winrt::check_hresult(
        device->CreateSamplerState(
            &samplerDesc,
            m_linearSampler.put()));

    // [11] Create a texture transformation for terrain by repeating the grass texture 
    // over the land mesh to get more resolution.
    // To tile the texture, we specify the WRAP address mode (which is a default value)
    // and scale the texture coordinates by 5 using a texture transformation matrix
    XMStoreFloat4x4(
        &m_grassTextureTransform,
        XMMatrixTranspose(XMMatrixScaling(5.0f, 5.0f, 0.0f)));

    // [12] Create rasterizer and blend states.
    m_stateManager->AddRasterizerState("NoCulling", RasterizerState::FillMode::Solid, RasterizerState::CullMode::CullNone, RasterizerState::WindingOrder::Clockwise);
    m_stateManager->AddBlendState("TransparentBlending", BlendState::Blending::Transparent);
   
    // Create light sources and materials.
    m_lightsController->CreateLights();
    m_materialController->CreateMaterials();

    // [Luna] The graph of a function y = f(x,z) is a grid in the xz-plane with the function y = f(x,z) 
    // applied to every point. The function makes the grid look like a terrain with hills and valleys.
    auto heightFunction = [this](float x, float z)->float 
    { 
        return this->GetHillHeight(x, z);
    };

    // The normal function calculates normals using partial derivatives of the height function.
    // Refer to [Luna] 7.13.3 Normal Computation (p.275) for details.
    auto normalFunction = [this](float x, float z)->XMFLOAT3
    {
        return this->GetHillNormal(x, z);
    };

    // Create a grid mesh with texture.
    m_terrainMesh->CreateWithTexture(160, 160, 50, 50, heightFunction, normalFunction);

    // Create a box.
    m_boxMesh->CreateWithTexture();

    // Inform other parts of the application that the initialization has completed.
    IsInitialized(true);
}

float const BlendingRenderer::GetHillHeight(float x, float z)
{
    return 0.3f * z * sinf(0.1f * x) + 0.3f * x * cosf(0.1f * z);
}

DirectX::XMFLOAT3 const BlendingRenderer::GetHillNormal(float x, float z)
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
        1.0f,
        -0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

    XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
    XMStoreFloat3(&n, unitNormal);

    return n;
}

/// <summary>
/// Device context dependent initialization.
/// </summary>
void BlendingRenderer::FinalizeInitialization()
{
    auto device{ m_deviceResources->GetD3DDevice() };
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Create a constant buffer for data that never changes.
    auto byteWidth = (sizeof(ConstantBufferNeverChanges) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC constantBufferDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(&constantBufferDesc, nullptr, m_constantBufferNeverChanges.put()));

    // Create a data structure for data that never changes.
    ConstantBufferNeverChanges constantBufferNeverChangesData;

    // Create the directional light.
    DirectionalLightDesc light;
    light.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    light.Diffuse = XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f);
    light.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    light.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
    constantBufferNeverChangesData.DirectionalLight = light;

    // Copy data that never changes to the appropriate constant buffer.
    context->UpdateSubresource(m_constantBufferNeverChanges.get(), 0, nullptr, &constantBufferNeverChangesData, 0, 0);
}

void BlendingRenderer::Update(float totalSeconds, float elapsedSeconds, DirectX::FXMVECTOR eyePosition, DirectX::FXMVECTOR lookingAtPosition)
{
    // Every quarter second, generate a random wave.
    static float t_base = 0.0f;
    if ((totalSeconds - t_base) >= 0.25f)
    {
        t_base += 0.25f;

        uint32_t i = 5 + rand() % 190;
        uint32_t j = 5 + rand() % 190;

        float r = MathHelper::RandF(1.0f, 2.0f);

        m_waves.Disturb(i, j, r);
    }

    m_waves.Update(elapsedSeconds);

    // Update the wave vertex buffer dynamically.
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    D3D11_MAPPED_SUBRESOURCE mappedData;
    winrt::check_hresult(
        context->Map(
            m_waveVertexBuffer.get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedData));

    VertexPositionTexture* v = reinterpret_cast<VertexPositionTexture*>(mappedData.pData);
    for (uint32_t i = 0; i < m_waves.VertexCount(); ++i)
    {
        v[i].Position = m_waves[i];
        v[i].Normal = m_waves.Normal(i);

        // Derive wave texture-coordinates in the range [0,1] from position.
        v[i].Texture.x = 0.5f + m_waves[i].x / m_waves.Width();
        v[i].Texture.y = 0.5f - m_waves[i].z / m_waves.Depth();
    }

    context->Unmap(m_waveVertexBuffer.get(), 0);

    // Animate water texture coordinates.

    // Tile water texture.
    XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

    // Scroll the water texture over the water geometry as a function of time.
    m_waterTextureOffset.y += 0.05f * elapsedSeconds;
    m_waterTextureOffset.x += 0.1f * elapsedSeconds;
    XMMATRIX wavesOffset = XMMatrixTranslation(m_waterTextureOffset.x, m_waterTextureOffset.y, 0.0f);

    // Combine scale and translation.
    XMStoreFloat4x4(&m_waterTextureTransform, wavesScale * wavesOffset);

    // Update the point light by circling it over the terrain.
    float pointLightPosX = 70.0f * cosf(0.2f * totalSeconds);
    float pointLightPosZ = 70.0f * sinf(0.2f * totalSeconds);
    float pointLightPosY = std::max(GetHillHeight(pointLightPosX, pointLightPosZ), -3.0f) + 10.0f;
    XMVECTOR pointLightPos = XMVectorSet(pointLightPosX, pointLightPosY, pointLightPosZ, 0.0f);
    m_lightsController->UpdatePointLight(pointLightPos);

    // Update the spot light by taking the camera position and aiming in the same direction the camera is looking.  
    // This way, it looks like we are holding a flashlight.
    m_lightsController->UpdateSpotLight(
        eyePosition, // the spot light's position
        XMVector3Normalize(lookingAtPosition - eyePosition)); // the spot light's direction
}

void BlendingRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Attach shaders.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.get(), nullptr, 0);

    // Get pointers to constant buffers.
    ID3D11Buffer* cbNeverChangesPtr{ m_constantBufferNeverChanges.get() };
    ID3D11Buffer* cbPerFramePtr{ m_constantBufferPerFrame.get() };
    ID3D11Buffer* cbPerObjectPtr{ m_constantBufferPerObject.get() };

    // Send the constant buffers to the graphics device.
    context->VSSetConstantBuffers(1, 1, &cbPerFramePtr);
    context->VSSetConstantBuffers(2, 1, &cbPerObjectPtr);
    context->PSSetConstantBuffers(0, 1, &cbNeverChangesPtr);
    context->PSSetConstantBuffers(1, 1, &cbPerFramePtr);
    context->PSSetConstantBuffers(2, 1, &cbPerObjectPtr);

    // Set the sampler.
    ID3D11SamplerState* pLinearSampler{ m_linearSampler.get() };
    context->PSSetSamplers(0, 1, &pLinearSampler);

    // [1] Draw the objects that do not require blending: the terrain and the box.
    RenderTerrain();
    RenderBox();

    // [2] Draw the transparent object: waves.
    RenderWaves();
}

void BlendingRenderer::RenderTerrain()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    ConstantBufferPerObject constantBufferPerObjectData;

    // Set the world matrix of the terrain.
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(XMMatrixIdentity()));

    // Set the material of the terrain.
    constantBufferPerObjectData.Material = m_materialController->GetTerrainMaterial();

    // Set the shader resource view i.e. a texture for terrain.
    ID3D11ShaderResourceView* pGrassTexture{ m_textures["grass"].get() };
    context->PSSetShaderResources(0, 1, &pGrassTexture);

    // Set the grass texture transform.
    XMStoreFloat4x4(
        &constantBufferPerObjectData.TextureTransform,
        XMMatrixTranspose(XMLoadFloat4x4(&m_grassTextureTransform)));

    // Send the per object cbuffer to GPU.
    context->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // Draw the terrain.
    m_terrainMesh->SetBuffers(sizeof(VertexPositionTexture));
    m_terrainMesh->Draw();
}

void BlendingRenderer::RenderBox()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    ConstantBufferPerObject constantBufferPerObjectData;

    // Set the world matrix of the box.
    XMStoreFloat4x4(&constantBufferPerObjectData.World, 
        XMMatrixTranspose(
            XMMatrixMultiply(
                XMMatrixScaling(18.0f, 18.0f, 18.0f), 
                XMMatrixTranslation(15.0f, 4.0f, -23.0f))));

    // Set the material of the box.
    constantBufferPerObjectData.Material = m_materialController->GetBoxMaterial();

    // Set the shader resource view i.e. a texture for the box.
    ID3D11ShaderResourceView* pBoxTexture{ m_textures["box"].get() };
    context->PSSetShaderResources(0, 1, &pBoxTexture);

    // Set the box texture transform to identity.
    XMStoreFloat4x4(
        &constantBufferPerObjectData.TextureTransform,XMMatrixIdentity());

    // Send the per object cbuffer to GPU.
    context->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // We need to disable back face culling because we can see through the wire texture of the box.
    m_stateManager->SetRasterizerState("NoCulling");

    // Draw the box.
    m_boxMesh->SetBuffers(sizeof(VertexPositionTexture));
    m_boxMesh->Draw();

    m_stateManager->DisableRasterizerState();
}

void BlendingRenderer::RenderWaves()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    ConstantBufferPerObject constantBufferPerObjectData;

    // Set the world matrix of the waves.
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(XMMatrixIdentity()));

    // Set the material of the waves.
    constantBufferPerObjectData.Material = m_materialController->GetWaveMaterial();

    // Set the shader resource view i.e. a texture for waves.
    ID3D11ShaderResourceView* pWaterTexture{ m_textures["water"].get() };
    context->PSSetShaderResources(0, 1, &pWaterTexture);

    // Set the water texture transform.
    XMStoreFloat4x4(
        &constantBufferPerObjectData.TextureTransform,
        XMMatrixTranspose(XMLoadFloat4x4(&m_waterTextureTransform)));

    // Send the per object cbuffer to GPU.
    context->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // Prepare the buffers for waves.
    UINT stride = sizeof(VertexPositionTexture);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_waveVertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_waveIndexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);

    // Set the transparency blending state.
    m_stateManager->SetBlendState("TransparentBlending");

    // Draw the waves.
    context->DrawIndexed(3 * m_waves.TriangleCount(), 0, 0);

    m_stateManager->DisableBlendState();
}

void BlendingRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_terrainMesh->ReleaseResources();
    m_boxMesh->ReleaseResources();
    m_waveVertexBuffer = nullptr;
    m_waveIndexBuffer = nullptr;
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void BlendingRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void BlendingRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, [[maybe_unused]] float totalSeconds)
{
    XMStoreFloat4x4(&m_constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    XMStoreFloat3(&m_constantBufferPerFrameData.EyePosition, eyePosition);

    // Copy the point light description to the per frame constant buffer.
    m_constantBufferPerFrameData.PointLight = m_lightsController->GetPointLight();

    // Copy the spot light description to the per frame constant buffer.
    m_constantBufferPerFrameData.SpotLight = m_lightsController->GetSpotLight();

    // Set fog parameters.
    m_constantBufferPerFrameData.FogColor = m_fogController->GetFogColor();
    m_constantBufferPerFrameData.FogStart = m_fogController->GetFogStart();
    m_constantBufferPerFrameData.FogRange = m_fogController->GetFogRange();

    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &m_constantBufferPerFrameData, 0, 0);
}

