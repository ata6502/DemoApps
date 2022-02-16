#pragma once

#include "IScissorTestRenderer.h"
#include "RendererBase.h"

enum class RendererType
{
    Color = 0,
    Texture = 1 // TODO: move texture renderer as default when finished
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources);
    static IScissorTestRenderer* GetScissorTestRenderer(RendererBase* renderer);
};

