#pragma once

#include "DeviceResources.h"
#include "SkySphere.h"

class SkyRenderer
{
public:
    SkyRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~SkyRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceResourcesAsync();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void Update(DirectX::FXMVECTOR eye, DirectX::FXMMATRIX viewMatrix);
    void Render();
    void ReleaseDeviceDependentResources();

private:
    std::shared_ptr<DX::DeviceResources>        m_deviceResources;

    winrt::com_ptr<ID3D11Buffer>                m_cbufferSky;
    winrt::com_ptr<ID3D11InputLayout>           m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>          m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>           m_pixelShader;
    winrt::com_ptr<ID3D11ShaderResourceView>    m_texture;
    winrt::com_ptr<ID3D11RasterizerState>       m_noCullRasterizerState;
    winrt::com_ptr<ID3D11DepthStencilState>     m_lessEqualDepthStencilState;

    bool                                        m_initialized;
    std::unique_ptr<SkySphere>                  m_skySphere;
};

