#pragma once

class RenderStates
{
public:
    explicit RenderStates();

    RenderStates(RenderStates&&) noexcept = default;
    RenderStates& operator= (RenderStates&&) noexcept = default;

    RenderStates(RenderStates const&) = delete;
    RenderStates& operator= (RenderStates const&) = delete;

    ~RenderStates();

    void CreateRenderStates(ID3D11Device3* device);
    void ReleaseDeviceDependentResources();

    ID3D11RasterizerState* GetWireframeRS() const;
    ID3D11RasterizerState* GetNoCullRS() const;
    ID3D11RasterizerState* GetCullClockwiseRS() const;

    ID3D11BlendState* GetAlphaToCoverageBS() const;
    ID3D11BlendState* GetTransparentBS() const;
    ID3D11BlendState* GetNoRenderTargetWritesBS() const;
    ID3D11BlendState* GetAdditiveBlendingBS() const;

    ID3D11DepthStencilState* GetDisableDepthBufferDSS() const;
    ID3D11DepthStencilState* GetDisableDepthBufferNoStencilDSS() const;
    ID3D11DepthStencilState* GetDrawReflectionDSS() const;
    ID3D11DepthStencilState* GetNoDoubleBlendDSS() const;
    ID3D11DepthStencilState* GetDepthComplexityCounterDSS() const;
    ID3D11DepthStencilState* GetDepthComplexityLevelDSS() const;

private:
    class RenderStatesImpl;

    std::unique_ptr<RenderStatesImpl> pImpl;
};

