#pragma once

#include "DeviceResources.h"
//#include "SceneRenderer.h" // TODO: create renderer
//#include "ShadowRenderer.h" // TODO: create renderer
#include "TextureMeshGenerator.h"

class MainRenderer
{
public:
    MainRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~MainRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceDependentResourcesAsync();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources(DirectX::FXMMATRIX projectionMatrix);
    void Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition);
    void Render();
    void ReleaseDeviceDependentResources();

    bool IsInitialized() const { return m_initialized; }

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<TextureMeshGenerator>   m_meshGenerator; // we share meshGenerator between renderers

    //std::unique_ptr<SceneRenderer>          m_sceneRenderer; // TODO: create renderer
    //std::unique_ptr<ShadowRenderer>         m_shadowRenderer; // TODO: create renderer

    bool                                    m_initialized;
};

