#pragma once

#include "RendererBase.h"

enum class RendererType
{
    Texture = 0,
    Mipmap = 1,
    Multitexture = 2,
    Material = 3,
    Color = 4
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
};

