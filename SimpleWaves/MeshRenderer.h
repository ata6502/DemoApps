#pragma once

#include "DeviceResources.h"
#include "GridMesh.h"
#include "RendererBase.h"

class MeshRenderer : public RendererBase
{
public:
    MeshRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~MeshRenderer() {}

    winrt::fire_and_forget InitializeInBackground();
    void Update(float totalSeconds, float elapsedSeconds, DirectX::FXMVECTOR eyePosition, DirectX::FXMVECTOR lookingAtPosition);
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    bool IsToonShaderSupported() const { return false; }
    bool AreLightParametersSupported() const { return false; }

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;

    DirectX::XMFLOAT4X4                     m_projMatrix;
    std::unique_ptr<GridMesh>               m_gridMesh;
};

