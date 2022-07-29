#pragma once

#include "DeviceResources.h"
#include "LightsController.h"
#include "MaterialController.h"
#include "RendererBase.h"

enum class RendererType
{
    Material = 0,
    Texture = 1,
    Blending = 2,
    Color = 3
};

class RendererFactory
{
public:
    static RendererBase* CreateRenderer(
        RendererType rendererType, 
        std::shared_ptr<DX::DeviceResources> const& deviceResources,
        std::shared_ptr<MaterialController> const& materialController,
        std::shared_ptr<LightsController> const& lightsController,
        std::shared_ptr<FogController> const& fogController);
};

