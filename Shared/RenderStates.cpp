#include "pch.h"
#include "RenderStates.h"

class RenderStates::RenderStatesImpl
{
public:
    RenderStatesImpl() :
        m_wireframeRS(nullptr),
        m_noCullRS(nullptr),
        m_cullClockwiseRS(nullptr),

        m_alphaToCoverageBS(nullptr),
        m_transparentBS(nullptr),
        m_noRenderTargetWritesBS(nullptr),
        m_additiveBlendingBS(nullptr),

        m_disableDepthBufferDSS(nullptr),
        m_disableDepthBufferNoStencilDSS(nullptr),
        m_drawReflectionDSS(nullptr),
        m_noDoubleBlendDSS(nullptr),
        m_depthComplexityCounterDSS(nullptr),
        m_depthComplexityLevelDSS(nullptr)
    { }

    void CreateWireframeRS(ID3D11Device3* device, D3D11_RASTERIZER_DESC const* wireframeDesc) { winrt::check_hresult(device->CreateRasterizerState(wireframeDesc, m_wireframeRS.put())); }
    void CreateNoCullRS(ID3D11Device3* device, D3D11_RASTERIZER_DESC const* noCullDesc) { winrt::check_hresult(device->CreateRasterizerState(noCullDesc, m_noCullRS.put())); }
    void CreateCullClockwiseRS(ID3D11Device3* device, D3D11_RASTERIZER_DESC const* cullClockwiseDesc) { winrt::check_hresult(device->CreateRasterizerState(cullClockwiseDesc, m_cullClockwiseRS.put())); }

    void CreateAlphaToCoverageBS(ID3D11Device3* device, D3D11_BLEND_DESC const* alphaToCoverageDesc) { winrt::check_hresult(device->CreateBlendState(alphaToCoverageDesc, m_alphaToCoverageBS.put())); }
    void CreateTransparentBS(ID3D11Device3* device, D3D11_BLEND_DESC const* transparentDesc) { winrt::check_hresult(device->CreateBlendState(transparentDesc, m_transparentBS.put())); }
    void CreateNoRenderTargetWritesBS(ID3D11Device3* device, D3D11_BLEND_DESC const* noRenderTargetWritesDesc) { winrt::check_hresult(device->CreateBlendState(noRenderTargetWritesDesc, m_noRenderTargetWritesBS.put())); }
    void CreateAdditiveBlendingBS(ID3D11Device3* device, D3D11_BLEND_DESC const* additiveBlendingDesc) { winrt::check_hresult(device->CreateBlendState(additiveBlendingDesc, m_additiveBlendingBS.put())); }

    void CreateDisableDepthBufferDSS(ID3D11Device3* device, D3D11_DEPTH_STENCIL_DESC const* disableDepthBufferDesc) { winrt::check_hresult(device->CreateDepthStencilState(disableDepthBufferDesc, m_disableDepthBufferDSS.put())); }
    void CreateDisableDepthBufferNoStencilDSS(ID3D11Device3* device, D3D11_DEPTH_STENCIL_DESC const* noDepthWritesAndStencilDesc) { winrt::check_hresult(device->CreateDepthStencilState(noDepthWritesAndStencilDesc, m_disableDepthBufferNoStencilDSS.put())); }
    void CreateDrawReflectionDSS(ID3D11Device3* device, D3D11_DEPTH_STENCIL_DESC const* drawReflectionDesc) { winrt::check_hresult(device->CreateDepthStencilState(drawReflectionDesc, m_drawReflectionDSS.put())); }
    void CreateNoDoubleBlendDSS(ID3D11Device3* device, D3D11_DEPTH_STENCIL_DESC const* noDoubleBlendDesc) { winrt::check_hresult(device->CreateDepthStencilState(noDoubleBlendDesc, m_noDoubleBlendDSS.put())); }
    void CreateDepthComplexityCounterDSS(ID3D11Device3* device, D3D11_DEPTH_STENCIL_DESC const* depthComlexityCounterDesc) { winrt::check_hresult(device->CreateDepthStencilState(depthComlexityCounterDesc, m_depthComplexityCounterDSS.put())); }
    void CreateDepthComplexityLevelDSS(ID3D11Device3* device, D3D11_DEPTH_STENCIL_DESC const* depthComplexityLevelDesc) { winrt::check_hresult(device->CreateDepthStencilState(depthComplexityLevelDesc, m_depthComplexityLevelDSS.put())); }

    ID3D11RasterizerState* GetWireframeRS() const { return m_wireframeRS.get(); }
    ID3D11RasterizerState* GetNoCullRS() const { return m_noCullRS.get(); }
    ID3D11RasterizerState* GetCullClockwiseRS() const { return m_cullClockwiseRS.get(); }

    ID3D11BlendState* GetAlphaToCoverageBS() const { return m_alphaToCoverageBS.get(); }
    ID3D11BlendState* GetTransparentBS() const { return m_transparentBS.get(); }
    ID3D11BlendState* GetNoRenderTargetWritesBS() const { return m_noRenderTargetWritesBS.get(); }
    ID3D11BlendState* GetAdditiveBlendingBS() const { return m_additiveBlendingBS.get(); }

    ID3D11DepthStencilState* GetDisableDepthBufferDSS() const { return m_disableDepthBufferDSS.get(); }
    ID3D11DepthStencilState* GetDisableDepthBufferNoStencilDSS() const { return m_disableDepthBufferNoStencilDSS.get(); }
    ID3D11DepthStencilState* GetDrawReflectionDSS() const { return m_drawReflectionDSS.get(); }
    ID3D11DepthStencilState* GetNoDoubleBlendDSS() const { return m_noDoubleBlendDSS.get(); }
    ID3D11DepthStencilState* GetDepthComplexityCounterDSS() const { return m_depthComplexityCounterDSS.get(); }
    ID3D11DepthStencilState* GetDepthComplexityLevelDSS() const { return m_depthComplexityLevelDSS.get(); }

    void ReleaseDeviceDependentResources()
    {
        m_wireframeRS = nullptr;
        m_noCullRS = nullptr;
        m_cullClockwiseRS = nullptr;

        m_alphaToCoverageBS = nullptr;
        m_transparentBS = nullptr;
        m_noRenderTargetWritesBS = nullptr;
        m_additiveBlendingBS = nullptr;

        m_disableDepthBufferDSS = nullptr;
        m_disableDepthBufferNoStencilDSS = nullptr;
        m_drawReflectionDSS = nullptr;
        m_noDoubleBlendDSS = nullptr;
        m_depthComplexityCounterDSS = nullptr;
        m_depthComplexityLevelDSS = nullptr;
    }

private:
    // Rasterizer states
    winrt::com_ptr<ID3D11RasterizerState> m_wireframeRS;
    winrt::com_ptr<ID3D11RasterizerState> m_noCullRS;
    winrt::com_ptr<ID3D11RasterizerState> m_cullClockwiseRS;

    // Blend states
    winrt::com_ptr<ID3D11BlendState> m_alphaToCoverageBS;
    winrt::com_ptr<ID3D11BlendState> m_transparentBS;
    winrt::com_ptr<ID3D11BlendState> m_noRenderTargetWritesBS;
    winrt::com_ptr<ID3D11BlendState> m_additiveBlendingBS;

    // Depth/stencil states
    winrt::com_ptr<ID3D11DepthStencilState> m_disableDepthBufferDSS;
    winrt::com_ptr<ID3D11DepthStencilState> m_disableDepthBufferNoStencilDSS;
    winrt::com_ptr<ID3D11DepthStencilState> m_drawReflectionDSS;
    winrt::com_ptr<ID3D11DepthStencilState> m_noDoubleBlendDSS;
    winrt::com_ptr<ID3D11DepthStencilState> m_depthComplexityCounterDSS;
    winrt::com_ptr<ID3D11DepthStencilState> m_depthComplexityLevelDSS;
};

RenderStates::RenderStates() :
    pImpl(new RenderStates::RenderStatesImpl())
{
}

RenderStates::~RenderStates()
{
}

void RenderStates::CreateRenderStates(ID3D11Device3* device)
{
    //
    // WireframeRS
    //
    D3D11_RASTERIZER_DESC wireframeDesc;
    ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
    wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireframeDesc.CullMode = D3D11_CULL_BACK;
    wireframeDesc.FrontCounterClockwise = false;
    wireframeDesc.DepthClipEnable = true;

    pImpl->CreateWireframeRS(device, &wireframeDesc);

    //
    // NoCullRS
    //
    D3D11_RASTERIZER_DESC noCullDesc;
    ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
    noCullDesc.FillMode = D3D11_FILL_SOLID;
    noCullDesc.CullMode = D3D11_CULL_NONE;
    noCullDesc.FrontCounterClockwise = false;
    noCullDesc.DepthClipEnable = true;

    pImpl->CreateNoCullRS(device, &noCullDesc);

    //
    // CullClockwiseRS
    //
    // We still cull backfaces by making front faces CCW. If we did not cull backfaces, 
    // then we would have to worry about the BackFace property in the D3D11_DEPTH_STENCIL_DESC.
    D3D11_RASTERIZER_DESC cullClockwiseDesc;
    ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
    cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
    cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
    cullClockwiseDesc.FrontCounterClockwise = true; // cull backfaces by making front faces CCW
    cullClockwiseDesc.DepthClipEnable = true;

    pImpl->CreateCullClockwiseRS(device, &cullClockwiseDesc);

    //
    // AlphaToCoverageBS
    //
    D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
    alphaToCoverageDesc.AlphaToCoverageEnable = true;
    alphaToCoverageDesc.IndependentBlendEnable = false;
    alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
    alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    pImpl->CreateAlphaToCoverageBS(device, &alphaToCoverageDesc);

    //
    // TransparentBS
    //
    D3D11_BLEND_DESC transparentDesc = { 0 };
    transparentDesc.AlphaToCoverageEnable = false;
    transparentDesc.IndependentBlendEnable = false;
    transparentDesc.RenderTarget[0].BlendEnable = true;
    transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

    // When alpha channels are opaque (=1.0) we don't need to care about alpha channel blending.
    transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    pImpl->CreateTransparentBS(device, &transparentDesc);

    //
    // NoRenderTargetWritesBS
    // 
    // This blend state blocks writes to the back buffer.
    D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
    noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
    noRenderTargetWritesDesc.IndependentBlendEnable = false;
    noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
    noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0; // no writes to any channel

    pImpl->CreateNoRenderTargetWritesBS(device, &noRenderTargetWritesDesc);

    //
    // AdditiveBlendingBS
    //
    D3D11_BLEND_DESC additiveBlendingDesc = { 0 };
    additiveBlendingDesc.AlphaToCoverageEnable = false;
    additiveBlendingDesc.IndependentBlendEnable = false;

    additiveBlendingDesc.RenderTarget[0].BlendEnable = true;
    additiveBlendingDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    additiveBlendingDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    additiveBlendingDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

    additiveBlendingDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    additiveBlendingDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    additiveBlendingDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    additiveBlendingDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    pImpl->CreateAdditiveBlendingBS(device, &additiveBlendingDesc);

    //
    // DisableDepthBufferDSS
    //
    // This depth/stencil state disables writes to the depth buffer. Otherwise an object, such as a mirror, would occlude the reflection.
    D3D11_DEPTH_STENCIL_DESC disableDepthBufferDesc;
    disableDepthBufferDesc.DepthEnable = true;
    disableDepthBufferDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // disable writes to the depth buffer
    disableDepthBufferDesc.DepthFunc = D3D11_COMPARISON_LESS;

    disableDepthBufferDesc.StencilEnable = true;
    disableDepthBufferDesc.StencilReadMask = 0xff;
    disableDepthBufferDesc.StencilWriteMask = 0xff;

    disableDepthBufferDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    disableDepthBufferDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    disableDepthBufferDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    disableDepthBufferDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // We are not rendering backfacing polygons, so these settings do not matter.
    disableDepthBufferDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    disableDepthBufferDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    disableDepthBufferDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    disableDepthBufferDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    pImpl->CreateDisableDepthBufferDSS(device, &disableDepthBufferDesc);

    //
    // DisableDepthBufferNoStencilDSS
    //
    D3D11_DEPTH_STENCIL_DESC noDepthWritesAndStencilDesc;
    noDepthWritesAndStencilDesc.DepthEnable = true;
    noDepthWritesAndStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // disable writes to the depth buffer
    noDepthWritesAndStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    noDepthWritesAndStencilDesc.StencilEnable = false; // do not use stencil

    // We are not using stencil, so these settings do not matter.
    noDepthWritesAndStencilDesc.StencilReadMask = 0xff;
    noDepthWritesAndStencilDesc.StencilWriteMask = 0xff;
    noDepthWritesAndStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    noDepthWritesAndStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    noDepthWritesAndStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    noDepthWritesAndStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    noDepthWritesAndStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    noDepthWritesAndStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    noDepthWritesAndStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    noDepthWritesAndStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    pImpl->CreateDisableDepthBufferNoStencilDSS(device, &noDepthWritesAndStencilDesc);

    //
    // DrawReflectionDSS
    //
    D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
    drawReflectionDesc.DepthEnable = true; // specify true to enable the depth buffering; specify false to disable it
    drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS; // usually the DepthFunc is D3D11_COMPARISON_LESS so that the usual depth test is performed
    drawReflectionDesc.StencilEnable = true;
    drawReflectionDesc.StencilReadMask = 0xff;
    drawReflectionDesc.StencilWriteMask = 0xff;

    drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    // We are not rendering backfacing polygons, so these settings do not matter.
    drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    pImpl->CreateDrawReflectionDSS(device, &drawReflectionDesc);

    //
    // NoDoubleBlendDSS
    //
    D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
    noDoubleBlendDesc.DepthEnable = true;
    noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
    noDoubleBlendDesc.StencilEnable = true;
    noDoubleBlendDesc.StencilReadMask = 0xff;
    noDoubleBlendDesc.StencilWriteMask = 0xff;

    noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    // We are not rendering backfacing polygons, so these settings do not matter.
    noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    pImpl->CreateNoDoubleBlendDSS(device, &noDoubleBlendDesc);

    //
    // DepthComplexityCounterDSS
    //
    // Uses the stencil buffer as a counter.
    D3D11_DEPTH_STENCIL_DESC depthComlexityCounterDesc;
    depthComlexityCounterDesc.DepthEnable = true;
    depthComlexityCounterDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthComlexityCounterDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthComlexityCounterDesc.StencilEnable = true;
    depthComlexityCounterDesc.StencilReadMask = 0xff;
    depthComlexityCounterDesc.StencilWriteMask = 0xff;

    // Increment the depth counter every time a pixel fragment is processed.
    // StencilDepthFailOp - Determines how the stencil buffer should be updated when the stencil test passes but the depth test fails for a pixel fragment.
    // StencilPassOp - Determines how the stencil buffer should be updated when the stencil test and depth test both pass for a pixel fragment.
    depthComlexityCounterDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthComlexityCounterDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthComlexityCounterDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    depthComlexityCounterDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // the stencil buffer entry should always be incremented for every pixel fragment no matter what

    depthComlexityCounterDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthComlexityCounterDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthComlexityCounterDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
    depthComlexityCounterDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    pImpl->CreateDepthComplexityCounterDSS(device, &depthComlexityCounterDesc);

    //
    // DepthComplexityLevelDSS
    //
    D3D11_DEPTH_STENCIL_DESC depthComplexityLevelDesc;
    depthComplexityLevelDesc.DepthEnable = true;
    depthComplexityLevelDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthComplexityLevelDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthComplexityLevelDesc.StencilEnable = true;
    depthComplexityLevelDesc.StencilReadMask = 0xff;
    depthComplexityLevelDesc.StencilWriteMask = 0xff;

    // Do not modify the stencil buffer.
    depthComplexityLevelDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthComplexityLevelDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthComplexityLevelDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthComplexityLevelDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    depthComplexityLevelDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthComplexityLevelDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthComplexityLevelDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthComplexityLevelDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    pImpl->CreateDepthComplexityLevelDSS(device, &depthComplexityLevelDesc);
}

void RenderStates::ReleaseDeviceDependentResources()
{
    pImpl->ReleaseDeviceDependentResources();
}

ID3D11RasterizerState* RenderStates::GetWireframeRS() const { return pImpl->GetWireframeRS(); }
ID3D11RasterizerState* RenderStates::GetNoCullRS() const { return pImpl->GetNoCullRS(); }
ID3D11RasterizerState* RenderStates::GetCullClockwiseRS() const { return pImpl->GetCullClockwiseRS(); }

ID3D11BlendState* RenderStates::GetAlphaToCoverageBS() const { return pImpl->GetAlphaToCoverageBS(); }
ID3D11BlendState* RenderStates::GetTransparentBS() const { return pImpl->GetTransparentBS(); }
ID3D11BlendState* RenderStates::GetNoRenderTargetWritesBS() const { return pImpl->GetNoRenderTargetWritesBS(); }
ID3D11BlendState* RenderStates::GetAdditiveBlendingBS() const { return pImpl->GetAdditiveBlendingBS(); }

ID3D11DepthStencilState* RenderStates::GetDisableDepthBufferDSS() const { return pImpl->GetDisableDepthBufferDSS(); }
ID3D11DepthStencilState* RenderStates::GetDisableDepthBufferNoStencilDSS() const { return pImpl->GetDisableDepthBufferNoStencilDSS(); }
ID3D11DepthStencilState* RenderStates::GetDrawReflectionDSS() const { return pImpl->GetDrawReflectionDSS(); }
ID3D11DepthStencilState* RenderStates::GetNoDoubleBlendDSS() const { return pImpl->GetNoDoubleBlendDSS(); }
ID3D11DepthStencilState* RenderStates::GetDepthComplexityCounterDSS() const { return pImpl->GetDepthComplexityCounterDSS(); }
ID3D11DepthStencilState* RenderStates::GetDepthComplexityLevelDSS() const { return pImpl->GetDepthComplexityLevelDSS(); }