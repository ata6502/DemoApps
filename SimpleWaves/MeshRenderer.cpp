#include "pch.h"

#include "ColorShaderStructures.h"
#include "MeshRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

MeshRenderer::MeshRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_gridMesh = std::make_unique<GridMesh>(deviceResources);

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction MeshRenderer::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"ColorVertexShader.cso");
    auto pixelShaderBytecode = co_await Utilities::ReadDataAsync(L"ColorPixelShader.cso");

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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

    // [7] Set the world matrix for the only object in the scene.
    ConstantBufferPerObject constantBufferPerObjectData;
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(XMMatrixIdentity()));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // [8] Create a rasterizer state.
    D3D11_RASTERIZER_DESC2 rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC2));
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    //rsDesc.FillMode = D3D11_FILL_SOLID; // TODO: Create a D3D11_FILL_SOLID rasterizer state
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.DepthClipEnable = true;

    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateRasterizerState2(
            &rsDesc,
            m_rasterizerState.put()));

    // [Luna] The graph of a function y = f(x,z) is a grid in the xz-plane with the function y = f(x,z) 
    // applied to every point. The function makes the grid look like a terrain with hills and valleys.
    auto heightFunction = [](float x, float z)->float { return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z)); };

    // Create a grid mesh with two colors: blue and red.
    m_gridMesh->Create(160, 160, 50, 50, heightFunction, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

    // Inform other parts of the application that the initialization has completed.
    IsInitialized(true);
}

void MeshRenderer::FinalizeInitialization()
{

}

void MeshRenderer::Update([[maybe_unused]] float totalSeconds, [[maybe_unused]] float elapsedSeconds)
{

}

void MeshRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    m_gridMesh->SetBuffers(sizeof(VertexPositionColor));

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(m_inputLayout.get());

    // Attach shaders.
    context->VSSetShader(m_vertexShader.get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.get(), nullptr, 0);

    // Get pointers to constant buffers.
    ID3D11Buffer* cbPerFramePtr{ m_constantBufferPerFrame.get() };
    ID3D11Buffer* cbPerObjectPtr{ m_constantBufferPerObject.get() };

    // Set the constant buffers.
    context->VSSetConstantBuffers(0, 1, &cbPerFramePtr);
    context->VSSetConstantBuffers(1, 1, &cbPerObjectPtr);
    context->PSSetConstantBuffers(0, 1, &cbPerFramePtr);

    // Set the rasterizer state.
    context->RSSetState(m_rasterizerState.get());

    // Draw the grid.
    m_gridMesh->Draw();
}

void MeshRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_gridMesh->ReleaseResources();
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
    m_rasterizerState = nullptr;
}

void MeshRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void MeshRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, [[maybe_unused]] DirectX::FXMVECTOR eyePosition, [[maybe_unused]] float totalSeconds)
{
    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}
