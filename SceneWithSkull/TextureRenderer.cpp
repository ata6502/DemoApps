#include "pch.h"
#include <string>

#include "DirectMathHelper.h"
#include "FileReader.h"
#include "LightsShaderStructures.h"
#include "TextureRenderer.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

const float TextureRenderer::DISTANCE_TO_CAMERA = 20.0f;
const float TextureRenderer::CAMERA_PITCH = 0.37f * XM_PI;

TextureRenderer::TextureRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr),
    m_rotation(0.0f)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    m_meshGenerator = std::make_unique<MeshGeneratorTexture>(deviceResources);

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction TextureRenderer::InitializeInBackground()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // [1] Load vertex shader bytecode.
    auto vertexShaderBytecode = co_await Utilities::ReadDataAsync(L"LightsVertexShader.cso");

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

    // [5] Create constant buffers.
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

    // [6] Load textures from files.
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\bricks.dds", m_textures["bricks"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\dummy.dds", m_textures["dummy"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\marble.dds", m_textures["marble"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\paint.dds", m_textures["paint"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\floor.dds", m_textures["stones"].put());
    co_await FileReader::LoadTextureAsync(device, L"Assets\\Textures\\wood.dds", m_textures["wood"].put());

    // [7] Create a sampler state.
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

    // [8] Create meshes using the MeshGenerator.
    co_await CreateMeshes();

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

    // Create the directional lights.

    // The three directional lights are:
    // - a primary light source
    // - a secondary fill light
    // - a back light.
    DirectionalLightDesc lights[3];

    lights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    lights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    lights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    lights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    lights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    lights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    lights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    lights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    lights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    lights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

    constantBufferNeverChangesData.DirectionalLightArray[0] = lights[0];
    constantBufferNeverChangesData.DirectionalLightArray[1] = lights[1];
    constantBufferNeverChangesData.DirectionalLightArray[2] = lights[2];

    // Copy directional lights to the constant buffer.
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

    // Render the scene.
    for (auto& obj : m_objects)
    {
        SetObjectData(obj.first, obj.second);
        m_meshGenerator->DrawMesh(obj.second.MeshName);
    }
}

void TextureRenderer::ReleaseResources()
{
    IsInitialized(false);
    m_meshGenerator->ReleaseBuffers();
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_constantBufferNeverChanges = nullptr;
    m_constantBufferPerFrame = nullptr;
    m_constantBufferPerObject = nullptr;
}

void TextureRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void TextureRenderer::Update(DirectX::FXMMATRIX viewMatrix, [[maybe_unused]] DirectX::FXMVECTOR eyePosition, float elapsedSeconds)
{
    m_rotation = (m_rotation + elapsedSeconds);
    if (m_rotation > XM_2PI)
        m_rotation = fmod(m_rotation, XM_2PI);

    ConstantBufferPerFrame constantBufferPerFrameData;
    XMStoreFloat4x4(&constantBufferPerFrameData.ViewProj,
        XMMatrixTranspose(viewMatrix * XMLoadFloat4x4(&m_projMatrix)));
    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerFrame.get(), 0, nullptr, &constantBufferPerFrameData, 0, 0);
}

void TextureRenderer::SetObjectData(std::string const& name, ObjectInfo const& info)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    ConstantBufferPerObject constantBufferPerObjectData;

    auto worldMatrix = XMLoadFloat4x4(&info.WorldMatrix);

    if (name == "Ellipsoid")
        worldMatrix = worldMatrix * XMMatrixRotationY(m_rotation) * XMMatrixTranslation(0.0f, 3.0f, -6.0f);

    // Calculate the world inverse transpose matrix in order to properly transform normals in case there are any non-uniform or shear transformations.
    auto worldInvTranspose = InverseTranspose(worldMatrix);

    XMStoreFloat4x4(&constantBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&constantBufferPerObjectData.WorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
    constantBufferPerObjectData.Material = info.Material;

    m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBufferPerObject.get(), 0, nullptr, &constantBufferPerObjectData, 0, 0);

    // Set the shader resource view i.e. a texture.
    ID3D11ShaderResourceView* pTexture{ m_textures[info.TextureName].get() };
    context->PSSetShaderResources(0, 1, &pTexture);
}

float TextureRenderer::GetDistanceToCamera()
{
    return DISTANCE_TO_CAMERA;
}

float TextureRenderer::GetCameraPitch()
{
    return CAMERA_PITCH;
}

winrt::Windows::Foundation::IAsyncAction TextureRenderer::CreateMeshes()
{
    m_meshGenerator->CreateGrid("grid", 20.0f, 30.0f, 60, 40);
    m_meshGenerator->CreateGeosphere("sphere", 1.0f, 4);
    m_meshGenerator->CreateCube("cube");
    m_meshGenerator->CreateCylinder("cylinder", 0.5f, 0.3f, 3.0f, 20, 10);
    co_await m_meshGenerator->CreateModelAsync("skull", L"Data\\skull.txt");

    m_meshGenerator->CreateBuffers();
}

void TextureRenderer::DefineSceneObjects()
{
    // Prepare materials.
    MaterialDesc materialGrid;
    materialGrid.Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    materialGrid.Diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    materialGrid.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 8.0f); // w = SpecularPower

    MaterialDesc materialCylinder;
    materialCylinder.Ambient = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
    materialCylinder.Diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
    materialCylinder.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 8.0f); // w = SpecularPower

    MaterialDesc materialSphere;
    materialSphere.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    materialSphere.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    materialSphere.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f); // w = SpecularPower

    MaterialDesc materialBox;
    materialBox.Ambient = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
    materialBox.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    materialBox.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 8.0f); // w = SpecularPower

    MaterialDesc materialSkull;
    materialSkull.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    materialSkull.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    materialSkull.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    ObjectInfo info;

    info.MeshName = "grid";
    info.Material = materialGrid;
    info.TextureName = "stones";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixIdentity());
    m_objects["Floor"] = info;

    info.MeshName = "sphere";
    info.Material = materialSphere;
    info.TextureName = "paint";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(1.0f, 2.0f, 2.0f));
    m_objects["Ellipsoid"] = info;

    info.MeshName = "cube";
    info.Material = materialBox;
    info.TextureName = "wood";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(3.0f, 1.0f, 3.0f) * XMMatrixTranslation(0.0f, 0.5f, -6.0f));
    m_objects["Stand"] = info;

    info.MeshName = "skull";
    info.Material = materialSkull;
    info.TextureName = "dummy";
    XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixTranslation(0.0f, 3.0f, 5.0f));
    m_objects["Skull"] = info;

    // Create 5 rows of 2 cylinders and 2 spheres per row.
    for (int i = 0; i < 5; ++i)
    {
        info.MeshName = "cylinder";
        info.Material = materialCylinder;
        info.TextureName = "bricks";
        auto n = std::to_string(i + 1);

        XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        m_objects["ColumnLeft" + n] = info;

        XMStoreFloat4x4(&info.WorldMatrix, XMMatrixTranslation(5.0f, 1.5f, -10.0f + i * 5.0f));
        m_objects["ColumnRight" + n] = info;

        info.MeshName = "sphere";
        info.TextureName = "marble";
        info.Material = materialSphere;

        XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        m_objects["BallLeft" + n] = info;

        XMStoreFloat4x4(&info.WorldMatrix, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, 3.5f, -10.0f + i * 5.0f));
        m_objects["BallRight" + n] = info;
    }
}
