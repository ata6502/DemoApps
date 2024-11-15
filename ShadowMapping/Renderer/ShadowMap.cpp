#include "pch.h"
#include "ShadowMap.h"

ShadowMap::ShadowMap(UINT width, UINT height) :
    m_width(width), 
    m_height(height), 
    m_depthMapSRV(nullptr),
    m_depthMapDSV(nullptr)
{
    // Configure the viewport to match the shadow map dimensions.
    m_viewport = { 0 };
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
}

ShadowMap::~ShadowMap()
{
    ReleaseDeviceDependentResources();
}

void ShadowMap::CreateDeviceDependentResources(ID3D11Device* device)
{
    // Create the texture of the specified dimensions.
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = m_width;
    texDesc.Height = m_height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    winrt::com_ptr<ID3D11Texture2D> depthMap{ nullptr };
    winrt::check_hresult(device->CreateTexture2D(&texDesc, nullptr, depthMap.put()));

    // Create views. The views keep a reference to the texture.
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    winrt::check_hresult(
        device->CreateDepthStencilView(
            depthMap.get(),
            &dsvDesc,
            m_depthMapDSV.put()));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    winrt::check_hresult(
        device->CreateShaderResourceView(
            depthMap.get(),
            &srvDesc,
            m_depthMapSRV.put()));
}

void ShadowMap::BindResources(ID3D11DeviceContext* context)
{
    context->RSSetViewports(1, &m_viewport);

    // Prepare the OM stage for rendering to the shadow map. Set a null render target.
    ID3D11RenderTargetView* renderTargets[1] = { nullptr };
    context->OMSetRenderTargets(1, renderTargets, m_depthMapDSV.get());
    context->ClearDepthStencilView(m_depthMapDSV.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMap::ReleaseDeviceDependentResources()
{
    m_depthMapSRV = nullptr;
    m_depthMapDSV = nullptr;
}
