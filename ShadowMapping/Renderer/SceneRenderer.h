#pragma once

#include "DeviceResources.h"
#include "SceneConstantBuffers.h"
#include "TextureMeshGenerator.h"

#include <unordered_map>

class SceneRenderer
{
public:
    SceneRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<TextureMeshGenerator> const& meshGenerator);
    ~SceneRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceDependentResourcesAsync();
    void FinalizeCreateDeviceResources();
    void SetProjectionMatrix(DirectX::FXMMATRIX projectionMatrix);
    void Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, DirectX::XMFLOAT4X4 shadowTransform, float rotation, float elapsedSeconds);
    DirectX::XMVECTOR UpdateLightDirection();
    void Render(ID3D11ShaderResourceView* shadowMapTexture);
    void ReleaseDeviceDependentResources();

    bool IsInitialized() const { return m_initialized; }

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<TextureMeshGenerator>   m_meshGenerator;

    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;
    winrt::com_ptr<ID3D11SamplerState>      m_linearSampler;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferPerObject;
    winrt::com_ptr<ID3D11SamplerState>      m_comparisonSampler; // used with PCF filtering

    bool                                    m_initialized;
    DirectX::XMFLOAT4X4                     m_projMatrix;
    DirectionalLightDesc                    m_directionalLight;
    std::unordered_map<std::string, winrt::com_ptr<ID3D11ShaderResourceView>> m_textures;
    std::unordered_map<std::string, MaterialDesc> m_materials;
    DirectX::XMFLOAT4X4                     m_floorTextureTransform;
    DirectX::XMFLOAT4X4                     m_columnTextureTransform;
    DirectX::XMFLOAT4X4                     m_shadowTransform;
    float                                   m_rotation;
    float                                   m_elapsedSeconds;

    // Variables used to animate the directional light.
    float                                   m_lightRotationAngle;
    DirectX::XMFLOAT3                       m_originalLightDirection;

    void CreateMaterials();
    void CreateTextureTransforms();
    void DrawScene();
    void DrawObject(DirectX::FXMMATRIX worldMatrix, DirectX::CXMMATRIX textureTransform, std::string materialName, std::string textureName, std::string meshName);
};

