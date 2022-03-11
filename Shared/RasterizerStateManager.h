#pragma once

#include "DeviceResources.h"

namespace RasterizerState
{
    enum class FillMode
    {
        Wireframe,
        Solid
    };

    enum class CullMode
    {
        CullNone, // disable backface culling
        CullFront,
        CullBack
    };

    enum class WindingOrder
    {
        Clockwise,
        CounterClockwise
    };
}

class RasterizerStateManager
{
public:
    RasterizerStateManager::RasterizerStateManager(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void AddRasterizerState(std::string name, RasterizerState::FillMode fillMode, RasterizerState::CullMode cullMode, RasterizerState::WindingOrder windingOrder);
    void SetRasterizerState(std::string name);
    void ReleaseResources();

private:
    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::map<std::string, winrt::com_ptr<ID3D11RasterizerState2>> m_rasterizerStates;
};

