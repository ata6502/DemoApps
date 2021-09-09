#include "pch.h"
#include "OneLightRenderer.h"
#include "Utilities.h"

using namespace DirectX;

OneLightRenderer::OneLightRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferData(),
    m_indexCount(0),
    m_initialized(false)
{
    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::fire_and_forget OneLightRenderer::InitializeInBackground()
{
    auto lifetime = get_strong();

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await ReadDataAsync(L"VertexShader.cso");
    auto pixelShaderBytecode = co_await ReadDataAsync(L"PixelShader.cso");

    // [2] Create vertex shader.
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateVertexShader(
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
        m_deviceResources->GetD3DDevice()->CreateInputLayout(
            vertexDesc,
            ARRAYSIZE(vertexDesc),
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            m_inputLayout.put()));

    // [5] Create the pixel shader.
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreatePixelShader(
            pixelShaderBytecode.data(),
            pixelShaderBytecode.Length(),
            nullptr,
            m_pixelShader.put()));

    // [6] Create a 16-byte aligned constant buffer.
    uint32_t byteWidth = (sizeof(ModelViewProjectionConstantBuffer) + 15) / 16 * 16;
    CD3D11_BUFFER_DESC constantBufferDesc(byteWidth, D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            m_constantBuffer.put()));

    // [7] Create cube vertices. Each vertex has a position and a normal vector.
    float n = 1.0f / sqrtf(3.0f); // all components of coordinates of the normals have the value n
    static const VertexPositionNormal cubeVertices[] =
    {
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-n, -n, -n) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(-n, -n,  n) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(-n,  n, -n) },
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(-n,  n,  n) },
        { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(n, -n, -n) },
        { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(n, -n,  n) },
        { XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(n,  n, -n) },
        { XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(n,  n,  n) }
    };

    // [8] Create vertex buffer and load data.
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = cubeVertices;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            m_vertexBuffer.put()));

    // [9] Create cube indices in the left-handed coordinate system.
    static const unsigned short cubeIndices[] =
    {
        0,1,2, // -x
        1,3,2,

        4,6,5, // +x
        5,6,7,

        0,5,1, // -y
        0,4,5,

        2,7,6, // +y
        2,3,7,

        0,6,4, // -z
        0,2,6,

        1,7,3, // +z
        1,5,7,
    };

    // [10] Keep the number of indices.
    m_indexCount = ARRAYSIZE(cubeIndices);

    // [11] Create index buffer and load indices to the buffer.
    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = cubeIndices;
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_indexBuffer.put()));

    // [12] Create the light and copy it to the constant buffer.
    DirectionalLight light;
    light.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    light.Diffuse = XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f);
    light.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    light.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
    m_constantBufferData.Light = light;

    // [13] Create the material and copy it to the constant buffer.
    MaterialDesc material;
    material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f); // w = SpecularPower
    m_constantBufferData.Material = material;

    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void OneLightRenderer::Render()
{
    if (m_initialized)
    {
        auto context{ m_deviceResources->GetD3DDeviceContext() };

        // Prepare the constant buffer to send it to the graphics device.
        context->UpdateSubresource(m_constantBuffer.get(), 0, nullptr, &m_constantBufferData, 0, 0);

        // Each vertex is one instance of the VertexPositionNormal struct.
        UINT stride = sizeof(VertexPositionNormal);
        UINT offset = 0;
        ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
        context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

        // Each index is one 16-bit unsigned integer (short).
        context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetInputLayout(m_inputLayout.get());

        // Attach our vertex shader.
        context->VSSetShader(m_vertexShader.get(), nullptr, 0);

        // Send the constant buffer to the graphics device.
        ID3D11Buffer* pConstantBuffer{ m_constantBuffer.get() };
        context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
        context->PSSetConstantBuffers(0, 1, &pConstantBuffer);

        // Attach our pixel shader.
        context->PSSetShader(m_pixelShader.get(), nullptr, 0);

        // Draw the cube.
        context->DrawIndexed(m_indexCount, 0, 0);
    }
}

void OneLightRenderer::ReleaseResources()
{
    m_initialized = false;
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

void OneLightRenderer::SetProjectionMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_constantBufferData.Projection,
        XMMatrixTranspose(projMatrix));
}

void OneLightRenderer::SetModelMatrix(DirectX::FXMMATRIX modelMatrix)
{
    XMStoreFloat4x4(&m_constantBufferData.Model,
        XMMatrixTranspose(modelMatrix));
}

void OneLightRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix)
{
    XMStoreFloat4x4(&m_constantBufferData.View,
        XMMatrixTranspose(viewMatrix));
}

void OneLightRenderer::SetEyePosition(DirectX::FXMVECTOR eyePosition)
{
    XMStoreFloat3(&m_constantBufferData.EyePosition, eyePosition);
}
