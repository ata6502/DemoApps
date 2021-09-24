#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Color = 0,
    Material = 1,
    Texture = 2
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

