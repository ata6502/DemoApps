#pragma once

#include "DeviceResources.h"
#include "ShaderStructures.h"

class OneLightRenderer : public winrt::implements<OneLightRenderer, winrt::Windows::Foundation::IInspectable>
{
public:
    OneLightRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::fire_and_forget InitializeInBackground();
    void Render();
    void ReleaseResources();

    void SetProjectionMatrix(DirectX::FXMMATRIX projMatrix);
    void SetModelMatrix(DirectX::FXMMATRIX modelMatrix);
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix);
    void SetEyePosition(DirectX::FXMVECTOR eyePosition);

private:
    std::shared_ptr<DX::DeviceResources>     m_deviceResources;

    // Direct3D resources for cube geometry.
    winrt::com_ptr<ID3D11InputLayout>        m_inputLayout;
    winrt::com_ptr<ID3D11Buffer>             m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>             m_indexBuffer;
    winrt::com_ptr<ID3D11VertexShader>       m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>        m_pixelShader;
    winrt::com_ptr<ID3D11Buffer>             m_constantBuffer;

    ModelViewProjectionConstantBuffer        m_constantBufferData;
    uint32_t                                 m_indexCount;
    bool                                     m_initialized;
};

