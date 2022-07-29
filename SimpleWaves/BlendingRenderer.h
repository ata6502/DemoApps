#pragma once

#include "BoxMesh.h"
#include "DeviceResources.h"
#include "FogController.h"
#include "GridMesh.h"
#include "LightsController.h"
#include "LightsShaderStructures.h"
#include "MaterialController.h"
#include "RendererBase.h"
#include "StateManager.h"
#include "Waves.h"

class BlendingRenderer : public RendererBase
{
public:
    BlendingRenderer(
        std::shared_ptr<DX::DeviceResources> const& deviceResources, 
        std::shared_ptr<MaterialController> const& materialController,
        std::shared_ptr<LightsController> const& lightsController,
        std::shared_ptr<FogController> const& fogController);
    ~BlendingRenderer() {}

    winrt::fire_and_forget InitializeInBackground();
    void Update(float totalSeconds, float elapsedSeconds, DirectX::FXMVECTOR eyePosition, DirectX::FXMVECTOR lookingAtPosition);
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    bool IsToonShaderSupported() const { return false; }
    bool AreLightParametersSupported() const { return true; }
    bool IsFogSupported() const { return true; }

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<MaterialController>     m_materialController;
    std::shared_ptr<LightsController>       m_lightsController;
    std::shared_ptr<FogController>       m_fogController;

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

    // Direct3D objects used with textures.
    std::map<std::string, winrt::com_ptr<ID3D11ShaderResourceView>> m_textures;
    winrt::com_ptr<ID3D11SamplerState>      m_linearSampler;

    // Texture transforms are used in the vertex shader to transform the input texture coordinates.
    // Texture coordinates are 2D points in texture plane. We can translate, rotate, and scale 
    // them like any other point.
    DirectX::XMFLOAT4X4                     m_grassTextureTransform;
    DirectX::XMFLOAT4X4                     m_waterTextureTransform;

    // Offset for water animation.
    DirectX::XMFLOAT2                       m_waterTextureOffset;

    DirectX::XMFLOAT4X4                     m_projMatrix;
    std::unique_ptr<GridMesh>               m_terrainMesh;
    std::unique_ptr<BoxMesh>                m_boxMesh;
    std::unique_ptr<StateManager>           m_stateManager;
    Waves                                   m_waves; // wave simulation

    // Helper functions.
    DirectX::XMFLOAT3 const GetHillNormal(float x, float z);
    float const GetHillHeight(float x, float z);
    void RenderTerrain();
    void RenderBox();
    void RenderWaves();
};

