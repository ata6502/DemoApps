#include "pch.h"

#include "ColorRenderer.h"
#include "MaterialRenderer.h"
#include "RendererFactory.h"

RendererBase* RendererFactory::CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources)
{
    switch (rendererType)
    {
    case RendererType::Color:
        return new ColorRenderer(deviceResources);
    case RendererType::Material:
        return new MaterialRenderer(deviceResources);
        break;
    case RendererType::Texture:
        // TODO: Create TextureRenderer
        break;
    default:
        return nullptr;
    }
}
