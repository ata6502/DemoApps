#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Texture = 0,
    Material = 1,
    Color = 2
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

