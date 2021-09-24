#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Material = 0,
    Color = 1,
    Texture = 2
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

