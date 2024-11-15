#pragma once

#include "DeviceResources.h"
#include "SceneRenderer.h"
#include "ShadowRenderer.h"
#include "TextureMeshGenerator.h"

class MainRenderer
{
public:
    MainRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);
    ~MainRenderer();

    winrt::Windows::Foundation::IAsyncAction CreateDeviceDependentResourcesAsync();
    void FinalizeCreateDeviceResources();
    void CreateWindowSizeDependentResources(DirectX::FXMMATRIX projectionMatrix);
    void Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float rotation, float elapsedSeconds);
    void Render();
    void ReleaseDeviceDependentResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<TextureMeshGenerator>   m_meshGenerator; // we share meshGenerator between renderers

    std::unique_ptr<SceneRenderer>          m_sceneRenderer;
    std::unique_ptr<ShadowRenderer>         m_shadowRenderer;

    bool                                    m_initialized;
};

