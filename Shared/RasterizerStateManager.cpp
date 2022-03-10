#include "pch.h"
#include "RasterizerStateManager.h"

RasterizerStateManager::RasterizerStateManager(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

void RasterizerStateManager::AddRasterizerState(RasterizerState state)
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // TODO: Convert from the RasterizerState enum to Direct3D enum.

    // Create a rasterizer state.
    D3D11_RASTERIZER_DESC2 rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC2));
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    //rsDesc.FillMode = D3D11_FILL_SOLID; // TODO: Create a D3D11_FILL_SOLID rasterizer state
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.DepthClipEnable = true;

    winrt::check_hresult(
        device->CreateRasterizerState2(
            &rsDesc,
            m_rasterizerStates[RasterizerState::Wireframe].put()));
}

void RasterizerStateManager::SetRasterizerState(RasterizerState state)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->RSSetState(m_rasterizerStates[state].get());
}

void RasterizerStateManager::ReleaseResources()
{
    // TODO: Should we release all rasterizer states?
}
