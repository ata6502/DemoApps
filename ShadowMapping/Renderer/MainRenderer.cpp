#include "pch.h"

#include <DirectXColors.h>

#include "MainRenderer.h"

using namespace DirectX;

MainRenderer::MainRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_initialized(false)
{
    m_meshGenerator = std::make_shared<TextureMeshGenerator>(m_deviceResources);

    // TODO: create renderers
    //m_sceneRenderer = std::make_unique<SceneRenderer>(m_deviceResources, m_meshGenerator);
    //m_shadowRenderer = std::make_unique<ShadowRenderer>(m_deviceResources, m_meshGenerator);
}

MainRenderer::~MainRenderer()
{
    ReleaseDeviceDependentResources();
}

// Create device-dependent resources.
winrt::Windows::Foundation::IAsyncAction MainRenderer::CreateDeviceDependentResourcesAsync()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // TODO: create renderers
    //co_await m_sceneRenderer->CreateDeviceDependentResourcesAsync();
    //co_await m_shadowRenderer->CreateDeviceDependentResourcesAsync();
    using namespace std::literals::chrono_literals;
    co_await winrt::resume_after(1s);

    // Create meshes.
    m_meshGenerator->CreateGrid("grid", 20.0f, 25.0f, 60, 40);
    m_meshGenerator->CreateCube("cube");
    m_meshGenerator->CreateCylinder("cylinder", 0.5f, 0.3f, 5.0f, 30, 20);
    m_meshGenerator->CreateGeosphere("sphere", 1.0f, 4);
    m_meshGenerator->CreateBuffers();
}

// Create context-dependent resources.
void MainRenderer::FinalizeCreateDeviceResources()
{
    // TODO: create renderers
    //m_sceneRenderer->FinalizeCreateDeviceResources();
    //m_shadowRenderer->FinalizeCreateDeviceResources();

    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void MainRenderer::CreateWindowSizeDependentResources(DirectX::FXMMATRIX projectionMatrix)
{
    // TODO: create renderers
    //m_sceneRenderer->CreateWindowSizeDependentResources(projectionMatrix);
    //m_shadowRenderer->CreateWindowSizeDependentResources(projectionMatrix);
}

void MainRenderer::Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // TODO: create renderers
    // 
    //XMVECTOR lightDirection = m_sceneRenderer->UpdateLightDirection();
    //XMFLOAT4X4 shadowTransform = m_shadowRenderer->BuildShadowTransform(lightDirection);

    //m_sceneRenderer->Update(viewMatrix, eyePosition, shadowTransform);
    //m_shadowRenderer->Update();
}

void MainRenderer::Render()
{
    if (!IsInitialized())
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Bind the vertex and index buffers containing mesh data.
    m_meshGenerator->SetBuffers();

    // TODO: create renderers
    //if (m_shadowRenderer->IsInitialized())
    //    m_shadowRenderer->Render();

    // Reset the viewport to target the whole screen.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Clear the views.
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthStencilView = m_deviceResources->GetDepthStencilView();
    context->ClearRenderTargetView(renderTargetView, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Bind the back buffer and the depth stencil view to the pipeline.
    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    // TODO: create renderers
    //if (m_sceneRenderer->IsInitialized() && m_shadowRenderer->IsInitialized())
    //{
    //    m_sceneRenderer->Render(m_shadowRenderer->GetShadowMapTexture());
    //}
}

void MainRenderer::ReleaseDeviceDependentResources()
{
    m_initialized = false;
    // TODO: create renderers
    //m_sceneRenderer->ReleaseDeviceDependentResources();
    //m_shadowRenderer->ReleaseDeviceDependentResources();
    m_meshGenerator->Clear();
}

