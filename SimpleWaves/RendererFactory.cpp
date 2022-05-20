#include "pch.h"

#include "MaterialController.h"
#include "MeshRenderer.h"
#include "RendererFactory.h"
#include "TextureRenderer.h"
#include "WaveRenderer.h"

RendererBase* RendererFactory::CreateRenderer(
    RendererType rendererType, 
    std::shared_ptr<DX::DeviceResources> const& deviceResources, 
    std::shared_ptr<MaterialController> const& materialController,
    std::shared_ptr<LightsController> const& lightsController)
{
    switch (rendererType)
    {
    case RendererType::Wave:
        return new WaveRenderer(deviceResources, materialController, lightsController);
    case RendererType::Texture:
        return new TextureRenderer(deviceResources, materialController, lightsController);
    case RendererType::Mesh:
        return new MeshRenderer(deviceResources);
    default:
        return nullptr;
    }
}
