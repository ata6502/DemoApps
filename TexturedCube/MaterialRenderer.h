#pragma once

#include "DeviceResources.h"
#include "RendererBase.h"

class MaterialRenderer : public RendererBase
{
public:
    MaterialRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    void SetWorldMatrix(DirectX::FXMMATRIX worldMatrix);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources for cube geometry.
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>            m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;

    uint32_t                                m_indexCount;
    DirectX::XMFLOAT4X4                     m_projMatrix;
};

