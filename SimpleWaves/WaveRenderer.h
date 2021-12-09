#pragma once

#include "DeviceResources.h"
#include "GridMesh.h"
#include "RendererBase.h"
#include "Waves.h"

class WaveRenderer : public RendererBase
{
public:
    WaveRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~WaveRenderer() {}

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Update(float totalSeconds, float elapsedSeconds);
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    void SetWorldMatrix(DirectX::FXMMATRIX worldMatrix);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>            m_waveVertexBuffer; // a dynamic vertex buffer
    winrt::com_ptr<ID3D11Buffer>            m_waveIndexBuffer;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerState;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;

    DirectX::XMFLOAT4X4                     m_projMatrix;
    std::unique_ptr<GridMesh>               m_gridMesh;
    Waves                                   m_waves; // wave simulation
};

