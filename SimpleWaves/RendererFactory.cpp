#include "pch.h"

#include "MeshRenderer.h"
#include "RendererFactory.h"
#include "WaveRenderer.h"

RendererBase* RendererFactory::CreateRenderer(RendererType rendererType, std::shared_ptr<DX::DeviceResources> const& deviceResources)
{
    switch (rendererType)
    {
    case RendererType::Wave:
        return new WaveRenderer(deviceResources);
    case RendererType::Mesh:
        return new MeshRenderer(deviceResources);
    default:
        return nullptr;
    }
}
