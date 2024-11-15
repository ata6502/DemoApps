#pragma once

#include "DeviceResources.h"
#include "ShadowMap.h"
#include "TextureMeshGenerator.h"

class ShadowRenderer
{
public:
    ShadowRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<TextureMeshGenerator> const& meshGenerator);
    ~ShadowRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceDependentResourcesAsync();
    void FinalizeCreateDeviceResources();
    void Update(float rotation);
    void Render();
    void ReleaseDeviceDependentResources();
    DirectX::XMFLOAT4X4 BuildShadowTransform(DirectX::FXMVECTOR lightDirection);
    ID3D11ShaderResourceView* GetShadowMapTexture() { return m_shadowMap->GetDepthMapSRV(); }

    bool IsInitialized() const { return m_initialized; }

private:
    // The resolution of the shadow map.
    const unsigned int ShadowMapWidth = 2048;
    const unsigned int ShadowMapHeight = 2048;

    struct BoundingSphere
    {
        BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
        DirectX::XMFLOAT3 Center;
        float Radius;
    };

    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<TextureMeshGenerator>   m_meshGenerator;

    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_cbufferPerObject;
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterStateDepthBias;

    bool                                    m_initialized;
    float                                   m_rotation;
    std::unique_ptr<ShadowMap>              m_shadowMap;
    DirectX::XMFLOAT4X4                     m_viewMatrix; // the light view matrix
    DirectX::XMFLOAT4X4                     m_projMatrix; // the light projection matrix
    DirectX::XMFLOAT4X4                     m_shadowTransform;
    BoundingSphere                          m_sceneBounds;

    void DrawSceneToShadowMap();
    void DrawObject(DirectX::FXMMATRIX worldMatrix, std::string meshName);
};

