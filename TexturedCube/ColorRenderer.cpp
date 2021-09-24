#include "pch.h"

#include "ColorRenderer.h"
#include "ColorShaderStructures.h"
#include "Utilities.h"

using namespace DirectX;

ColorRenderer::ColorRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_indexCount(0)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction ColorRenderer::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await ReadDataAsync(L"ColorVertexShader.cso");
    auto pixelShaderBytecode = co_await ReadDataAsync(L"ColorPixelShader.cso");

    // [2] Create vertex shader.
    winrt::check_hresult(
        device->CreateVertexShader(
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            nullptr,
            m_vertexShader.put()));

    // [3] Create vertex description.
    // Hook up the position element to the input slot 0 and the color element to the input slot 1.
    // Note that the AlignedByteOffset parameter is 0 for both elements.
    static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
    m_constantBufferPerFrame = nullptr;
    winrt::check_hresult(
        device->CreateBuffer(&bd, nullptr, m_constantBufferPerFrame.put()));

    bd.ByteWidth = (sizeof(ConstantBufferPerObject) + 15) / 16 * 16;
    m_constantBufferPerObject = nullptr;
    winrt::check_hresult(
        device->CreateBuffer(&bd, nullptr, m_constantBufferPerObject.put()));

    // Use two vertex buffers (and two input slots) to feed the pipeline with vertices, 
    // one that stores the position element and the other that stores the color element.
    // This is not necessary and it's here just as an exercise ([Luna] 6.15.2)
 
    // [7] Create cube vertex positions. 
    static const VertexPosition cubeVertexPositions[] =
    {
        {XMFLOAT3(-0.5f, -0.5f, -0.5f)},
        {XMFLOAT3(-0.5f, -0.5f,  0.5f)},
        {XMFLOAT3(-0.5f,  0.5f, -0.5f)},
        {XMFLOAT3(-0.5f,  0.5f,  0.5f)},
        {XMFLOAT3( 0.5f, -0.5f, -0.5f)},
        {XMFLOAT3( 0.5f, -0.5f,  0.5f)},
        {XMFLOAT3( 0.5f,  0.5f, -0.5f)},
        {XMFLOAT3( 0.5f,  0.5f,  0.5f)}
    };

    // [8] Create a vertex buffer for cube vertices.
    m_vertexBufferPosition.attach(CreateImmutableVertexBuffer(device, sizeof(cubeVertexPositions), &cubeVertexPositions));

    // [9] Create cube vertex colors.
    static const VertexColor cubeVertexColors[] =
    {
        {XMFLOAT3(0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(0.0f, 1.0f, 0.0f)},
        {XMFLOAT3(0.0f, 1.0f, 1.0f)},
        {XMFLOAT3(1.0f, 0.0f, 0.0f)},
        {XMFLOAT3(1.0f, 0.0f, 1.0f)},
        {XMFLOAT3(1.0f, 1.0f, 0.0f)},
        {XMFLOAT3(1.0f, 1.0f, 1.0f)}
    };

    // [10] Create a vertex buffer for cube colors.
    m_vertexBufferColor.attach(CreateImmutableVertexBuffer(device, sizeof(cubeVertexColors), &cubeVertexColors));

    // [11] Create cube indices in the left-handed coordinate system.
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

    // Inform other parts of the application that the initialization has completed.
    IsInitialized(true);
}

/// <summary>
/// Device context dependent initialization.
/// </summary>
void ColorRenderer::FinalizeInitialization()
{
}

void ColorRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Set both vertex buffers.
    ID3D11Buffer* vertexBuffers[] = { m_vertexBufferPosition.get(), m_vertexBufferColor.get() };
    UINT strides[] = { sizeof(VertexPosition), sizeof(VertexColor) };
    UINT offsets[] = { 0, 0 };
    context->IASetVertexBuffers(
        0,             // the input slot in which to start binding vertex buffers
        2,             // the number of vertex buffers we are binding to the input slots
        vertexBuffers, // a pointer to the first element of an array of vertex buffers
        strides,       // a pointer to the first element of an array of strides (one for each vertex buffer)
        offsets        // a pointer to the first element of an array of offsets (one for each vertex buffer)
    );

    // Each index is one 16-bit unsigned integer (short).
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Attach shaders.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.get(), nullptr, 0);

    // Get pointers to constant buffers.
    ID3D11Buffer* cbPerFramePtr{ m_constantBufferPerFrame.get() };
    ID3D11Buffer* cbPerObjectPtr{ m_constantBufferPerObject.get() };

    // Send the constant buffers to the graphics device.
    context->VSSetConstantBuffers(0, 1, &cbPerFramePtr);
    context->VSSetConstantBuffers(1, 1, &cbPerObjectPtr);

    // Draw the cube.
    context->DrawIndexed(m_indexCount, 0, 0);
}

void ColorRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_vertexBufferPosition = nullptr;
    m_vertexBufferColor = nullptr;
    m_indexBuffer = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void ColorRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void ColorRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, [[maybe_unused]] DirectX::FXMVECTOR eyePosition, float totalSeconds)
{
    // The eye position is not used in the ColorRenderer shaders.

    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    XMStoreFloat4(&constantBufferPerFrameData.Time, XMVectorSet(totalSeconds, 0, 0, 0)); // we use only the x-member in the shader
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}

void ColorRenderer::SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
{
    ConstantBufferPerObject constantBufferPerObjectData;
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);
}
