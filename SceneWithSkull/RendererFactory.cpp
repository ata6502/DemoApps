#include "pch.h"

#include "ColorRenderer.h"
#include "RendererFactory.h"
#include "TextureRenderer.h"

RendererBase* RendererFactory::CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources)
{
    switch (rendererType)
    {
    case RendererType::Color:
        return new ColorRenderer(deviceResources);
    case RendererType::Texture:
        return new TextureRenderer(deviceResources);
    default:
        return nullptr;
    }
}
