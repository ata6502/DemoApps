#include "pch.h"

#include "LightsShaderStructures.h"
#include "TextureRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

TextureRenderer::TextureRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr),
    m_outputSize()
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_meshGenerator = std::make_unique<MeshGeneratorTexture>(deviceResources);

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction TextureRenderer::InitializeInBackground()
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

void TextureRenderer::Render()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    m_meshGenerator->SetBuffers();

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

    // Render the scene.
    for (auto& obj : m_objects)
    {
        SetObjectData(XMLoadFloat4x4(&obj.WorldMatrix));
        m_meshGenerator->DrawMesh(obj.MeshName);
    }
}

void TextureRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_meshGenerator->ReleaseBuffers();
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void TextureRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void TextureRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, [[maybe_unused]] DirectX::FXMVECTOR eyePosition, [[maybe_unused]] float totalSeconds)
{
    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}

void TextureRenderer::SetObjectData(DirectX::FXMMATRIX worldMatrix)
{
    ConstantBufferPerObject constantBufferPerObjectData;
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

    // TODO: Create the material per object.
    MaterialDesc material;
    material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f); // w = SpecularPower
    constantBufferPerObjectData.Material = material;

    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);
}

void TextureRenderer::SetOutputSize(winrt::Windows::Foundation::Size outputSize)
{
    m_outputSize = outputSize;
    SetScissorTestRectangle();
}

void TextureRenderer::CreateMeshes()
{
    m_meshGenerator->CreateCube("cube");

    m_meshGenerator->CreateBuffers();
}

void TextureRenderer::DefineSceneObjects()
{
    ObjectInfo info;

    info.MeshName = "cube";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixIdentity());
    m_objects.push_back(info);
}

/// <summary>
/// Sends an array of screen rectangles (in this example, there is only one rectangle in the array)
/// to the Direct3D scissor test. The scissor test discards all pixels outside the scissor rectangles.
/// </summary>
void TextureRenderer::EnableScissorTest(bool enabled)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Set the rasterizer state.
    if (enabled)
        context->RSSetState(m_rasterizerStateScissorTestEnabled.get());
    else 
        context->RSSetState(m_rasterizerStateScissorTestDisabled.get());
}

void TextureRenderer::SetScissorTestLeftRightMargin(float marginPercent)
{
    m_leftRightMarginPercent = marginPercent;
    SetScissorTestRectangle();
}

void TextureRenderer::SetScissorTestTopBottomMargin(float marginPercent)
{
    m_topBottomMarginPercent = marginPercent;
    SetScissorTestRectangle();
}

void TextureRenderer::SetScissorTestRectangle()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    auto leftRightMargin = static_cast<long>((m_leftRightMarginPercent / 100.0f) * m_outputSize.Width);
    auto topBottomMargin = static_cast<long>((m_topBottomMarginPercent / 100.0f) * m_outputSize.Width);

    // Set the scissor test rectangle.
    D3D11_RECT rects = { 
        leftRightMargin, 
        topBottomMargin, 
        static_cast<long>(m_outputSize.Width) - leftRightMargin, 
        static_cast<long>(m_outputSize.Height) - topBottomMargin };

    context->RSSetScissorRects(1, &rects);
}