#pragma once

#include "ConstantBuffers.h"
#include "DeviceResources.h"
#include "TextureMeshGenerator.h"

class SceneRenderer
{
public:
    SceneRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~SceneRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceResourcesAsync();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix);
    void PrepareRender();
    void ReleaseDeviceDependentResources();

    // Mesh methods.
    void CreateSphereMesh(std::string const& name, float radius, uint16_t subdivisionCount);
    void CreateCylinderMesh(std::string const& name, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount);
    void CreateCubeMesh(std::string const& name);
    void CreateGridMesh(std::string const& name, float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth);
    void FinalizeCreateMeshes();

    // Rendering methods.
    void RenderMesh(std::string const& name);

    // World matrix methods.
    void SetWorldMatrix(DirectX::FXMMATRIX worldMatrix);

    // Material methods.
    void AddMaterial(std::string const& name, MaterialDesc const& material);
    void SetMaterial(std::string const& name);

    // Texture methods.
    winrt::Windows::Foundation::IAsyncAction AddTexture(std::string const& name, std::wstring const& path);
    void SetTexture(std::string const& name);
    void SetTextureTransform(DirectX::FXMMATRIX textureTransform);

    // Blend state methods.
    void SetTransparentBlendState();
    void ClearTransparentBlendState();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    bool                                    m_initialized;
    std::unique_ptr<TextureMeshGenerator>   m_meshGenerator;

    // Direct3D resources.
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferPerObject;
    std::map<std::string, winrt::com_ptr<ID3D11ShaderResourceView>> m_textures;
    winrt::com_ptr<ID3D11BlendState>        m_transparentBlendState;

    // Data structures.
    CBufferPerObject                        m_cbufferPerObjectData;
    std::map<std::string, MaterialDesc>     m_materials;
};

