#pragma once

#include "DeviceResources.h"

// A utility class that stores the scene depth from the perspective of the light source.
class ShadowMap
{
public:
    explicit ShadowMap(UINT width, UINT height);
    ~ShadowMap();

    void CreateDeviceDependentResources(ID3D11Device* device);
    void ReleaseDeviceDependentResources();

    // Binds the depth/stencil buffer and sets the null render target.
    void BindResources(ID3D11DeviceContext* context);

    // Provides access to the shader resource view of the shadow map.
    ID3D11ShaderResourceView* GetDepthMapSRV() { return m_depthMapSRV.get(); }

private:
    ShadowMap(ShadowMap&&) = delete;
    ShadowMap& operator= (ShadowMap&&) = delete;

    ShadowMap(ShadowMap const&) = delete;
    ShadowMap& operator= (ShadowMap const&) = delete;

    UINT m_width;
    UINT m_height;
    winrt::com_ptr<ID3D11ShaderResourceView> m_depthMapSRV;
    winrt::com_ptr<ID3D11DepthStencilView> m_depthMapDSV;
    D3D11_VIEWPORT m_viewport;
};
