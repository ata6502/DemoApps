#include "pch.h"
#include "RasterizerStateManager.h"

RasterizerStateManager::RasterizerStateManager(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

void RasterizerStateManager::AddRasterizerState(std::string name, RasterizerState::FillMode fillMode, RasterizerState::CullMode cullMode, RasterizerState::WindingOrder windingOrder)
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

void RasterizerStateManager::SetRasterizerState(std::string name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->RSSetState(m_rasterizerStates[name].get());
}

void RasterizerStateManager::ReleaseResources()
{
    for (auto& state : m_rasterizerStates)
        state.second = nullptr;
}
