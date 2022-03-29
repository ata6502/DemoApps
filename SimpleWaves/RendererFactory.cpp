#include "pch.h"

#include "MaterialController.h"
#include "MeshRenderer.h"
#include "RendererFactory.h"
#include "WaveRenderer.h"

RendererBase* RendererFactory::CreateRenderer(
    RendererType rendererType, 
    std::shared_ptr<DX::DeviceResources> const& deviceResources, 
    std::shared_ptr<MaterialController> const& materialController)
{
    switch (rendererType)
    {
    case RendererType::Wave:
        return new WaveRenderer(deviceResources, materialController);
    case RendererType::Mesh:
        return new MeshRenderer(deviceResources);
    default:
        return nullptr;
    }
}
