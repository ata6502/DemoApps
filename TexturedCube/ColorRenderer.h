#pragma once

#include "ColorShaderStructures.h"
#include "DeviceResources.h"

class ColorRenderer : public winrt::implements<ColorRenderer, winrt::Windows::Foundation::IInspectable>
{
public:
    ColorRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::fire_and_forget InitializeInBackground();
    void Render();
    void ReleaseResources();

    void OnResize(DirectX::FXMMATRIX projMatrix);
    void OnUpdate(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition);
    void SetModelMatrix(DirectX::FXMMATRIX modelMatrix);

private:
    std::shared_ptr<DX::DeviceResources>     m_deviceResources;

    // Direct3D resources for cube geometry.
    winrt::com_ptr<ID3D11InputLayout>        m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>             m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>             m_indexBuffer;
    winrt::com_ptr<ID3D11VertexShader>       m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>        m_pixelShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>             m_constantBufferOnResize;
    winrt::com_ptr<ID3D11Buffer>             m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>             m_constantBufferPerObject;

    // Constant buffer data.
    ConstantBufferOnResize                   m_constantBufferOnResizeData;
    ConstantBufferPerFrame                   m_constantBufferPerFrameData;
    ConstantBufferPerObject                  m_constantBufferPerObjectData;

    uint32_t                                 m_indexCount;
    bool                                     m_initialized;
};

