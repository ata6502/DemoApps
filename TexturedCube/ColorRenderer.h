#pragma once

#include "DeviceResources.h"
#include "RendererBase.h"

/*
    ColorRenderer uses two vertex buffers (and two input slots) to feed the pipeline with vertices:
    - One vertex buffer stores the position element.
    - Another vertex buffer stores the color element.
*/
class ColorRenderer : public RendererBase
{
public:
    ColorRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    void SetWorldMatrix(DirectX::FXMMATRIX worldMatrix);

private:
    std::shared_ptr<DX::DeviceResources>     m_deviceResources;

    // Direct3D resources for cube geometry. 
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>            m_vertexBufferPosition; // a vertex buffer to keep positions
    winrt::com_ptr<ID3D11Buffer>            m_vertexBufferColor; // a vertex buffer to keep colors
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;

    uint32_t                                m_indexCount;
    DirectX::XMFLOAT4X4                     m_projMatrix;
};

