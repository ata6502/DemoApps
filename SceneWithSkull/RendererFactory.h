#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Texture = 0,
    Color = 1
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

