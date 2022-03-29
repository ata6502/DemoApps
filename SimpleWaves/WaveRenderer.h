#pragma once

#include "DeviceResources.h"
#include "GridMesh.h"
#include "LightsShaderStructures.h"
#include "MaterialController.h"
#include "RendererBase.h"
#include "Waves.h"

class WaveRenderer : public RendererBase
{
public:
    WaveRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<MaterialController> const& materialController);
    ~WaveRenderer() {}

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Update(float totalSeconds, float elapsedSeconds, DirectX::FXMVECTOR eyePosition, DirectX::FXMVECTOR lookingAtPosition);
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    bool IsToonShaderSupported() const { return true; }
    bool IsSpecularComponentSupported() const { return true; }

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<MaterialController>     m_materialController;

    // Direct3D resources.
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>            m_waveVertexBuffer; // a dynamic vertex buffer
    winrt::com_ptr<ID3D11Buffer>            m_waveIndexBuffer;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;
    ConstantBufferPerFrame                  m_constantBufferPerFrameData;

    DirectX::XMFLOAT4X4                     m_projMatrix;
    std::unique_ptr<GridMesh>               m_terrainMesh;
    Waves                                   m_waves; // wave simulation

    // Keep the description of a point light source as a data member as we need to change its position every frame.
    PointLightDesc m_pointLight;

    // Keep the description of a spot light source as a data member as we need to change its position and direction every frame.
    SpotLightDesc m_spotLight;

    // Helper functions.
    DirectX::XMFLOAT3 const GetHillNormal(float x, float z);
    float const GetHillHeight(float x, float z);
};

