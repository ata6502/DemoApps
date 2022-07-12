#include "pch.h"
#include "StateManager.h"

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

void StateManager::AddBlendState(std::string name, BlendState::Blending blending)
{
    using namespace BlendState;

    auto device{ m_deviceResources->GetD3DDevice() };

    // Create a blend state.
    D3D11_BLEND_DESC bsDesc;
    ZeroMemory(&bsDesc, sizeof(D3D11_BLEND_DESC));

    bsDesc.AlphaToCoverageEnable = blending == Blending::AlphaToCoverage;
    bsDesc.IndependentBlendEnable = false;

    switch (blending)
    {
    case Blending::Transparent:
        bsDesc.RenderTarget[0].BlendEnable = true;

        bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

        // TODO: LT: Alpha channels are completely opaque for land and waves (=1.0) hence we don't really need to care about alpha channel blending.
        bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
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

    bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    switch (blending)
    {
    case Blending::AlphaToCoverage:
    case Blending::Transparent:
        m_blendFactors[name] = { 0.f, 0.f, 0.f, 0.f };
        break;
    case Blending::Test:
        m_blendFactors[name] = { 1.f, 0.f, 0.f, 0.f }; // (*)
        break;
    default:
        m_blendFactors[name] = { 0.f, 0.f, 0.f, 0.f };
        break;
    }

    winrt::check_hresult(
        device->CreateBlendState(
            &bsDesc,
            m_blendStates[name].put()));
}

void StateManager::SetBlendState(std::string name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    auto blendFactor = m_blendFactors[name];
    context->OMSetBlendState(m_blendStates[name].get(), blendFactor.Factors, 0xffffffff);
}

void StateManager::ReleaseResources()
{
    for (auto& state : m_rasterizerStates)
        state.second = nullptr;
}
