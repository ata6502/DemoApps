#include "pch.h"

#include "ColorShaderStructures.h"
#include "SceneRenderer.h"
#include "Utilities.h"

using namespace DirectX;

SceneRenderer::SceneRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr),
    m_initialized(false)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_meshGenerator = std::make_unique<MeshGenerator>(deviceResources);

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction SceneRenderer::InitializeInBackground()
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

    // [7] Create rasterizer states to enable or diable the scissor test using 
    // the D3D11_RASTERIZER_DESC::ScissorEnable flag.
    D3D11_RASTERIZER_DESC2 rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC2));
    rsDesc.AntialiasedLineEnable = false;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.DepthClipEnable = true;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.MultisampleEnable = false;
    rsDesc.ScissorEnable = true; // enable the scissor test
    rsDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state to enable the scissor test.
    winrt::check_hresult(
        device->CreateRasterizerState2(&rsDesc, m_rasterizerStateScissorTestEnabled.put()));

    // Create the rasterizer state to disable the scissor test.
    rsDesc.ScissorEnable = false; // disable the scissor test
    winrt::check_hresult(
        device->CreateRasterizerState2(&rsDesc, m_rasterizerStateScissorTestDisabled.put()));

    // [8] Create meshes using the MeshGenerator.
    CreateMeshes();

    // [9] Define the scene.
    DefineSceneObjects();

    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void SceneRenderer::FinalizeInitialization()
{
}

void SceneRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    m_meshGenerator->SetBuffers();

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
    context->PSSetConstantBuffers(0, 1, &cbPerFramePtr);

    // Render the scene.
    for (auto& obj : m_objects)
    {
        SetWorldMatrix(XMLoadFloat4x4(&obj.WorldMatrix));
        m_meshGenerator->DrawMesh(obj.MeshName);
    }
}

void SceneRenderer::ReleaseResources()
{
    m_initialized = false;
    m_meshGenerator->ReleaseBuffers();
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void SceneRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void SceneRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds)
{
    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}

void SceneRenderer::SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
{
    ConstantBufferPerObject constantBufferPerObjectData;
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);
}

void SceneRenderer::SetOutputSize(winrt::Windows::Foundation::Size outputSize)
{
    const float LeftRightMarginPercent = 0.3;
    const float UpDownMarginPercent = 0.2;

    long leftRightMargin = static_cast<long>(LeftRightMarginPercent * outputSize.Width);
    long upDownMargin = static_cast<long>(UpDownMarginPercent * outputSize.Height);

    // Set the scissor test rectangle.
    D3D11_RECT rects = { leftRightMargin, upDownMargin, outputSize.Width - leftRightMargin, outputSize.Height - upDownMargin };
    m_deviceResources->GetD3DDeviceContext()->RSSetScissorRects(1, &rects);
}

void SceneRenderer::CreateMeshes()
{
    m_meshGenerator->CreateCylinder("cylinder", 0.2f, 0.1f, 1.0f, 16, 2);
    m_meshGenerator->CreateCube("cube");
    m_meshGenerator->CreateGeosphere("geosphere", 1.0f, 3);
    m_meshGenerator->CreatePyramid("pyramid");
    m_meshGenerator->CreateSphere("sphere", 1.0f, 20, 10);
    m_meshGenerator->CreateGrid("grid", 4, 3, 4, 4);

    m_meshGenerator->CreateBuffers();
}

void SceneRenderer::DefineSceneObjects()
{
    ObjectInfo info;

    info.MeshName = "cylinder";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(0.5f, 0.5f, -0.5f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(-0.5f, 0.5f, -0.5f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(0.5f, 0.5f, 0.5f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(-0.5f, 0.5f, 0.5f));
    m_objects.push_back(info);

    info.MeshName = "cube";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(1.3f, 0.18f, 1.3f) * XMMatrixRotationZ(XM_PI) * XMMatrixTranslation(0.0f, 1.09f, 0.0f));
    m_objects.push_back(info);

    info.MeshName = "geosphere";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
    m_objects.push_back(info);

    info.MeshName = "pyramid";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.0f, 1.4f, 0.0f));
    m_objects.push_back(info);

    info.MeshName = "sphere";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.0f, 0.19f, 0.0f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.3f, 0.19f, 0.0f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.6f, 0.19f, 0.0f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.15f, 0.19f, -0.2f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.45f, 0.19f, 0.2f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.15f, 0.19f, 0.2f));
    m_objects.push_back(info);
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(-1.45f, 0.19f, -0.2f));
    m_objects.push_back(info);

    info.MeshName = "grid";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(0.0f, -0.01f, 0.0f));
    m_objects.push_back(info);
}

/// <summary>
/// Sends an array of screen rectangles (in this example, there is only one rectangle in the array)
/// to the Direct3D scissor test. The scissor test discards all pixels outside the scissor rectangles.
/// </summary>
void SceneRenderer::EnableScissorTest(bool enabled)
{
    // Set the rasterizer state.
    if (enabled)
        m_deviceResources->GetD3DDeviceContext()->RSSetState(m_rasterizerStateScissorTestEnabled.get());
    else 
        m_deviceResources->GetD3DDeviceContext()->RSSetState(m_rasterizerStateScissorTestDisabled.get());
}
