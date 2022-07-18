#include "pch.h"
#include "StateManager.h"

// Direct3D states are organized in state blocks: RasterizerState, BlendState, etc.
// - Each state has a default state. For example, a default blend state is "blending disabled".
// - Calling a Set method with null, restores the default state. For example: OMSetBlendState(null)
// - Create state blocks at application initialization, and then just switch between the states as needed.

StateManager::StateManager(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

void StateManager::AddRasterizerState(std::string name, RasterizerState::FillMode fillMode, RasterizerState::CullMode cullMode, RasterizerState::WindingOrder windingOrder)
{
    using namespace RasterizerState;

    auto device{ m_deviceResources->GetD3DDevice() };

    // Create a rasterizer state.
    D3D11_RASTERIZER_DESC2 rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC2));

    rsDesc.AntialiasedLineEnable = false;

    // Convert from the RasterizerState::FillMode to Direct3D enum.
    switch (cullMode)
    {
    case CullMode::CullNone:
        rsDesc.CullMode = D3D11_CULL_NONE;
        break;
    case CullMode::CullFront:
        rsDesc.CullMode = D3D11_CULL_FRONT;
        break;
    case CullMode::CullBack:
        rsDesc.CullMode = D3D11_CULL_BACK;
        break;
    }

    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.DepthClipEnable = true;

    // Convert from the RasterizerState::FillMode to Direct3D enum.
    switch (fillMode)
    {
    case FillMode::Wireframe:
        rsDesc.FillMode = D3D11_FILL_WIREFRAME;
        break;
    case FillMode::Solid:
        rsDesc.FillMode = D3D11_FILL_SOLID;
        break;
    }

    // Convert from the RasterizerState::WindingOrder to Direct3D enum.
    switch (windingOrder)
    {
    case WindingOrder::Clockwise:
        rsDesc.FrontCounterClockwise = false;
        break;
    case WindingOrder::CounterClockwise:
        rsDesc.FrontCounterClockwise = true;
        break;
    }

    rsDesc.MultisampleEnable = false;
    rsDesc.ScissorEnable = false;
    rsDesc.SlopeScaledDepthBias = 0.0f;

    winrt::check_hresult(
        device->CreateRasterizerState2(
            &rsDesc,
            m_rasterizerStates[name].put()));
}

void StateManager::SetRasterizerState(std::string name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->RSSetState(m_rasterizerStates[name].get());
}

void StateManager::DisableRasterizerState()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->RSSetState(nullptr);
}

void StateManager::AddBlendState(std::string name, BlendState::Blending blending)
{
    using namespace BlendState;

    auto device{ m_deviceResources->GetD3DDevice() };

    // Create a blend state.
    D3D11_BLEND_DESC bsDesc;
    ZeroMemory(&bsDesc, sizeof(D3D11_BLEND_DESC));

    // Alpha-to-coverage is a multisampling technique used to render textures such as foliage or gates.
    // It requires multisampling to be enabled i.e., the back and depth buffer need to specify multisampling.
    bsDesc.AlphaToCoverageEnable = blending == Blending::AlphaToCoverage;

    // Used with multiple render targets. Direct3D 11 supports rendering to up to 8 render targets simultaneously. 
    // When this flag is set to true, each render target can have different blend factors, operations, etc. 
    // If it is false, all the render targets are blended the same way as described by the first element in 
    // the D3D11_BLEND_DESC::RenderTarget array.  
    bsDesc.IndependentBlendEnable = false;

    switch (blending)
    {
    case Blending::Transparent:
        // RenderTarget is an array of eight D3D11_RENDER_TARGET_BLEND_DESC elements, where the i-th element describes 
        // how blending is done for the i-th render target.

        // True enables blending; false disables it.
        bsDesc.RenderTarget[0].BlendEnable = true;

        // The source blend factor for RGB blending.
        bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;

        // The destination blend factor for RGB blending.
        bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

        // The RGB blending operator.
        bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

        // For Blending::Transparent we assume that alpha channels of all objects are opaque (=1.0)
        // Because of that we just set default values for alpha channel blending.

        // The destination blend factor for alpha blending.
        bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

        // The destination blend factor for alpha blending.
        bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

        // The alpha blending operator.
        bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        break;
    case Blending::Test:
        bsDesc.RenderTarget[0].BlendEnable = true;

        bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR; // defined in (*)
        bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO; // do not add the destination pixel color
        bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

        bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        break;
    default:
        bsDesc.RenderTarget[0].BlendEnable = false;
        break;
    }

    // Set up color channels in the back buffer that are written to after blending. For example, to disable 
    // writes to the RGB channels, and only write to the alpha channel, specify D3D11_COLOR_WRITE_ENABLE_ALPHA.
    bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    switch (blending)
    {
    case Blending::AlphaToCoverage:
    case Blending::Transparent:
        m_blendFactor[name] = { 0.f, 0.f, 0.f, 0.f };
        break;
    case Blending::Test:
        m_blendFactor[name] = { 1.f, 0.f, 0.f, 0.f }; // (*)
        break;
    default:
        m_blendFactor[name] = { 0.f, 0.f, 0.f, 0.f };
        break;
    }

    winrt::check_hresult(
        device->CreateBlendState(
            &bsDesc,
            m_blendStates[name].put()));
}

// Note: blending requires per-pixel work. Enable it only when you need it, and turn it
// off when you are done.
void StateManager::SetBlendState(std::string name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    const auto& blendFactor = m_blendFactor[name];

    // Bind a blend state object to the output merger stage.
    context->OMSetBlendState(
        m_blendStates[name].get(),      // a pointer to a blend state to enable
        blendFactor.Factor,             // an array of four floats defining an RGBA color vector used as a blend factor when D3D11_BLEND_BLEND_FACTOR or D3D11_BLEND_INV_BLEND_FACTOR is specified
        0xffffffff);                    // 32-bit sample mask (multisampling can take up to 32 samples); 0xffffffff does not disable any samples
}

void StateManager::DisableBlendState()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    UINT sampleMask = 0xffffffff;

    context->OMSetBlendState(nullptr, blendFactor, sampleMask);
}

void StateManager::ReleaseResources()
{
    for (auto& state : m_rasterizerStates)
        state.second = nullptr;
}
