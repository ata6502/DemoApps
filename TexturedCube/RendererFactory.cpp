#include "pch.h"

#include "ColorRenderer.h"
#include "MaterialRenderer.h"
#include "RendererFactory.h"
#include "TextureRenderer.h"

RendererBase* RendererFactory::CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources)
{
    switch (rendererType)
    {
    case RendererType::Texture:
        return new TextureRenderer(deviceResources, TextureRendererMode::Normal);
    case RendererType::Mipmap:
        return new TextureRenderer(deviceResources, TextureRendererMode::Mipmap);
    case RendererType::Material:
        return new MaterialRenderer(deviceResources);
    case RendererType::Color:
        return new ColorRenderer(deviceResources);
    default:
        return nullptr;
    }
}
