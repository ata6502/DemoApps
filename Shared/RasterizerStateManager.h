#pragma once

#include "DeviceResources.h"

enum class RasterizerState
{
    Default,
    Wireframe,          // FillMode
    Solid,              // FillMode
    Clockwise,          // Front
    CounterClockwise,   // Front
    CullNone,           // CullMode - disable backface culling
    CullFront,          // CullMode
    CullBack            // CullMode
};

class RasterizerStateManager
{
public:
    RasterizerStateManager::RasterizerStateManager(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void AddRasterizerState(RasterizerState state);
    void SetRasterizerState(RasterizerState state);
    void ReleaseResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::map<RasterizerState, winrt::com_ptr<ID3D11RasterizerState2>> m_rasterizerStates;
};

