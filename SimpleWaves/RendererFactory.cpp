#include "pch.h"

#include "BlendingRenderer.h"
#include "ColorRenderer.h"
#include "MaterialRenderer.h"
#include "RendererFactory.h"
#include "TextureRenderer.h"

RendererBase* RendererFactory::CreateRenderer(
    RendererType rendererType, 
    std::shared_ptr<DX::DeviceResources> const& deviceResources, 
    std::shared_ptr<MaterialController> const& materialController,
    std::shared_ptr<LightsController> const& lightsController,
    std::shared_ptr<FogController> const& fogController)
{
    switch (rendererType)
    {
    case RendererType::Material:
        return new MaterialRenderer(deviceResources, materialController, lightsController);
    case RendererType::Texture:
        return new TextureRenderer(deviceResources, materialController, lightsController);
    case RendererType::Blending:
        return new BlendingRenderer(deviceResources, materialController, lightsController, fogController);
    case RendererType::Color:
        return new ColorRenderer(deviceResources);
    default:
        return nullptr;
    }
}
