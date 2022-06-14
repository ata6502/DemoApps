#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Texture = 0,
    Mipmap = 1,
    Material = 2,
    Color = 3
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

