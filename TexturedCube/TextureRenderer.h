#pragma once

#include "DeviceResources.h"
#include "RendererBase.h"

enum class TextureRendererMode
{
    Normal,
    Mipmap,
    Multitexture,
    PageFlipping
};

class TextureRenderer : public RendererBase
{
public:
    TextureRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, TextureRendererMode mode);
    ~TextureRenderer() {}

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

    // Direct3D objects used with textures.
    // In multitexturing, texture1 is used for a flare and texture2 for alpha.
    winrt::com_ptr<ID3D11ShaderResourceView> m_texture1;
    winrt::com_ptr<ID3D11ShaderResourceView> m_texture2;
    winrt::com_ptr<ID3D11SamplerState>      m_linearSampler;

    // Variables used with page flipping animation.
    // Note that working with individual texture animation frames one-by-one is inefficient. 
    // It would be better to use a texture atlas, and then offset the texture coordinates every 
    // 1/30th of a second to the next frame of the animation.
    std::vector<winrt::com_ptr<ID3D11ShaderResourceView>> m_sequence;
    uint32_t m_currentPageInSequence; // from 0 to 119

    uint32_t                                m_indexCount;
    DirectX::XMFLOAT4X4                     m_projMatrix;
    TextureRendererMode                     m_mode;
};

