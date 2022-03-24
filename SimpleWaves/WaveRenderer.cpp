#include "pch.h"

#include "LightsShaderStructures.h"
#include "MathHelper.h"
#include "WaveRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

WaveRenderer::WaveRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_terrainMesh = std::make_unique<GridMesh>(deviceResources);

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction WaveRenderer::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"LightsVertexShader.cso");
    auto pixelShaderBytecode = co_await Utilities::ReadDataAsync(L"LightsPixelShader.cso");

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
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
    vertexBufferDesc.ByteWidth = sizeof(VertexPositionNormal) * m_waves.VertexCount();
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

    // [9] Create materials
    m_terrainMaterial.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
    m_terrainMaterial.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
    m_terrainMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f); // w = SpecularPower

    m_waveMaterial.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
    m_waveMaterial.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
    m_waveMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f); // w = SpecularPower

    // [10] Create the point light source.
    m_pointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_pointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_pointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_pointLight.Attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_pointLight.Range = 25.0f;

    // [11] Create the spot light source.
    m_spotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_spotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
    m_spotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_spotLight.Attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_spotLight.Spot = 128.0f;
    m_spotLight.Range = 10000.0f;

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

    // Create a grid mesh with two colors: blue and red.
    m_terrainMesh->Create(160, 160, 50, 50, heightFunction, normalFunction);

    // Inform other parts of the application that the initialization has completed.
    IsInitialized(true);
}

float const WaveRenderer::GetHillHeight(float x, float z)
{
    return 0.3f * z * sinf(0.1f * x) + 0.3f * x * cosf(0.1f * z);
}

DirectX::XMFLOAT3 const WaveRenderer::GetHillNormal(float x, float z)
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
void WaveRenderer::FinalizeInitialization()
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

// Animate waves.
void WaveRenderer::Update(float totalSeconds, float elapsedSeconds, DirectX::FXMVECTOR eyePosition, DirectX::FXMVECTOR lookingAtPosition)
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

    VertexPositionNormal* v = reinterpret_cast<VertexPositionNormal*>(mappedData.pData);
    for (uint32_t i = 0; i < m_waves.VertexCount(); ++i)
    {
        v[i].Position = m_waves[i];
        v[i].Normal = m_waves.Normal(i);
    }

    context->Unmap(m_waveVertexBuffer.get(), 0);

    // Update the point light by circling it over the terrain.
    m_pointLight.Position.x = 70.0f * cosf(0.2f * totalSeconds);
    m_pointLight.Position.z = 70.0f * sinf(0.2f * totalSeconds);
    m_pointLight.Position.y = std::max(GetHillHeight(m_pointLight.Position.x, m_pointLight.Position.z), -3.0f) + 10.0f;

    // Update the spot light by taking the camera position and aiming in the same direction the camera is looking.  
    // This way, it looks like we are holding a flashlight.
    XMStoreFloat3(&m_spotLight.Position, eyePosition);
    XMStoreFloat3(&m_spotLight.Direction, XMVector3Normalize(lookingAtPosition - eyePosition));
}

void WaveRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Attach shaders. A pixel changed is attached in ShaderController.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);

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

    ConstantBufferPerObject constantBufferPerObjectData;

    // Set the material and the world matrix of the terrain.
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(XMMatrixIdentity()));
    constantBufferPerObjectData.Material = m_terrainMaterial;
    context->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // Draw the terrain.
    m_terrainMesh->SetBuffers(sizeof(VertexPositionNormal));
    m_terrainMesh->Draw();

    // Set the material and the world matrix of the waves.
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(XMMatrixIdentity()));
    constantBufferPerObjectData.Material = m_waveMaterial;
    context->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // Draw the waves.
    UINT stride = sizeof(VertexPositionNormal);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_waveVertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_waveIndexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(3 * m_waves.TriangleCount(), 0, 0);
}

void WaveRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_terrainMesh->ReleaseResources();
    m_waveVertexBuffer = nullptr;
    m_waveIndexBuffer = nullptr;
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void WaveRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void WaveRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, [[maybe_unused]] float totalSeconds)
{
    XMStoreFloat4x4(&m_constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    XMStoreFloat3(&m_constantBufferPerFrameData.EyePosition, eyePosition);

    // Copy the point light description to the per frame constant buffer.
    m_constantBufferPerFrameData.PointLight = m_pointLight;

    // Copy the spot light description to the per frame constant buffer.
    m_constantBufferPerFrameData.SpotLight = m_spotLight;

    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &m_constantBufferPerFrameData, 0, 0);
}
