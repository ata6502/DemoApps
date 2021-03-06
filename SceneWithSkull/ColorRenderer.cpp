#include "pch.h"

#include "ColorRenderer.h"
#include "ColorShaderStructures.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

const float ColorRenderer::DISTANCE_TO_CAMERA = 3.0f;
const float ColorRenderer::CAMERA_PITCH = 0.3f * XM_PI;

ColorRenderer::ColorRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_meshGenerator = std::make_unique<MeshGenerator>(deviceResources);

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::fire_and_forget ColorRenderer::InitializeInBackground()
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

    // [7] Create meshes using the MeshGenerator.
    CreateMeshes();

    // [8] Define the scene.
    DefineSceneObjects();

    // Inform other parts of the application that the initialization has completed.
    IsInitialized(true);
}

void ColorRenderer::FinalizeInitialization()
{
}

void ColorRenderer::Render()
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
        SetObjectData(XMLoadFloat4x4(&obj.WorldMatrix));
        m_meshGenerator->DrawMesh(obj.MeshName);
    }
}

void ColorRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_meshGenerator->Clear();
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void ColorRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void ColorRenderer::Update(DirectX::FXMMATRIX viewMatrix, [[maybe_unused]] DirectX::FXMVECTOR eyePosition, [[maybe_unused]] float elapsedSeconds)
{
    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}

void ColorRenderer::SetObjectData(DirectX::FXMMATRIX worldMatrix)
{
    ConstantBufferPerObject constantBufferPerObjectData;
    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);
}

float ColorRenderer::GetDistanceToCamera()
{
    return DISTANCE_TO_CAMERA;
}

float ColorRenderer::GetCameraPitch()
{
    return CAMERA_PITCH;
}

void ColorRenderer::CreateMeshes()
{
    m_meshGenerator->CreateCylinder("cylinder", 0.2f, 0.1f, 1.0f, 16, 2);
    m_meshGenerator->CreateCube("cube");
    m_meshGenerator->CreateGeosphere("geosphere", 1.0f, 3);
    m_meshGenerator->CreatePyramid("pyramid");
    m_meshGenerator->CreateSphere("sphere", 1.0f, 20, 10);
    m_meshGenerator->CreateGrid("grid", 4, 3, 4, 4);

    m_meshGenerator->CreateBuffers();
}

void ColorRenderer::DefineSceneObjects()
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
