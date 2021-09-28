#include "pch.h"

#include "FileReader.h"
#include "TextureRenderer.h"
#include "TextureShaderStructures.h"
#include "Utilities.h"

using namespace DirectX;

TextureRenderer::TextureRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_indexCount(0),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr),
    m_crateTexture(nullptr)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction TextureRenderer::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await FileReader::ReadDataAsync(L"TextureVertexShader.cso");
    auto pixelShaderBytecode = co_await FileReader::ReadDataAsync(L"TexturePixelShader.cso");

    // [2] Create vertex shader.
    winrt::check_hresult(
        device->CreateVertexShader(
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            nullptr,
            m_vertexShader.put()));

    // [3] The input layout describes position, normal, and texture coordinates.
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

    // [7] Create cube vertices. Each vertex has a position, a normal, and texture coordinates.
    // Also, we define a 24 vertices instead of 8 vertices. This allows us to specify more 
    // accurate normal vectors.
    float l = 0.5f, n = 1.0f;
    static const VertexPositionTexture cubeVertices[] =
    {
        // front face
        { XMFLOAT3(-l, -l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-l, +l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(+l, +l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(+l, -l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(1.0f, 1.0f) },

        // back face
        { XMFLOAT3(-l, -l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(+l, -l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(+l, +l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-l, +l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(1.0f, 0.0f) },

        // top face
        { XMFLOAT3(-l, +l, -l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-l, +l, +l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(+l, +l, +l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(+l, +l, -l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 1.0f) },

        // bottom face
        { XMFLOAT3(-l, -l, -l), XMFLOAT3(0, -n, 0), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(+l, -l, -l), XMFLOAT3(0, -n, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(+l, -l, +l), XMFLOAT3(0, -n, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-l, -l, +l), XMFLOAT3(0, -n, 0), XMFLOAT2(1.0f, 0.0f) },

        // left face
        { XMFLOAT3(-l, -l, +l), XMFLOAT3(-n, 0, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-l, +l, +l), XMFLOAT3(-n, 0, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-l, +l, -l), XMFLOAT3(-n, 0, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-l, -l, -l), XMFLOAT3(-n, 0, 0), XMFLOAT2(1.0f, 1.0f) },

        // right face
        { XMFLOAT3(+l, -l, -l), XMFLOAT3(n, 0, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(+l, +l, -l), XMFLOAT3(n, 0, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(+l, +l, +l), XMFLOAT3(n, 0, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(+l, -l, +l), XMFLOAT3(n, 0, 0), XMFLOAT2(1.0f, 1.0f) }
    };

    // [8] Create an immutable vertex buffer.
    m_vertexBuffer.attach(CreateImmutableVertexBuffer(device, sizeof(cubeVertices), &cubeVertices));

    // [11] Create cube indices in the left-handed coordinate system.
    static const unsigned short cubeIndices[] =
    {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 5, 6,
        4, 6, 7,

        // top face
        8, 9, 10,
        8, 10, 11,

        // bottom face
        12, 13, 14,
        12, 14, 15,

        // left face
        16, 17, 18,
        16, 18, 19,

        // right face
        20, 21, 22,
        20, 22, 23
    };

    // [12] Keep the number of indices.
    m_indexCount = ARRAYSIZE(cubeIndices);

    // [13] Create index buffer and load indices to the buffer.
    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = cubeIndices;
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        device->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_indexBuffer.put()));

    // [14] Load a texture from a file and create the shader resource view to the texture.
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\crate.dds", m_crateTexture.put());

    // [15] Create a sampler state.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
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

    // Inform other parts of the application that the initialization has completed.
    IsInitialized(true);
}

/// <summary>
/// Device context dependent initialization.
/// </summary>
void TextureRenderer::FinalizeInitialization()
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

    // Create two directional lights.
    DirectionalLight light1;
    light1.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    light1.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    light1.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
    light1.Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
    constantBufferNeverChangesData.LightArray[0] = light1;

    DirectionalLight light2;
    light2.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    light2.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
    light2.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
    light2.Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);
    constantBufferNeverChangesData.LightArray[1] = light2;

    // Create the material.
    MaterialDesc material;
    material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f); // w = SpecularPower
    constantBufferNeverChangesData.Material = material;

    // Copy data that never changes to the appropriate constant buffer.
    context->UpdateSubresource(m_constantBufferNeverChanges.get(), 0, nullptr, &constantBufferNeverChangesData, 0, 0);
}

void TextureRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Each vertex is one instance of the VertexPositionTexture struct.
    UINT stride = sizeof(VertexPositionTexture);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    // Each index is one 16-bit unsigned integer (short).
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

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

    // Set the sampler.
    ID3D11SamplerState* pLinearSampler{ m_linearSampler.get() };
    context->PSSetSamplers(0, 1, &pLinearSampler);

    // Set the shader resource view i.e. our texture.
    ID3D11ShaderResourceView* pCrateTexture{ m_crateTexture.get() };
    context->PSSetShaderResources(0, 1, &pCrateTexture);

    // Draw the cube.
    context->DrawIndexed(m_indexCount, 0, 0);
}

void TextureRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void TextureRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void TextureRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, [[maybe_unused]] float totalSeconds)
{
    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    XMStoreFloat3(&constantBufferPerFrameData.EyePosition, eyePosition);
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}

void TextureRenderer::SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
{
    ConstantBufferPerObject constantBufferPerObjectData;
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);
}
