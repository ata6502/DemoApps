#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Texture = 0,
    Mipmap = 1,
    Multitexture = 2,
    PageFlipping = 3,
    Material = 4,
    Color = 5
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

