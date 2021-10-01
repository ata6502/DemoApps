#include "pch.h"

#include "SceneRenderer.h"

using namespace DirectX;

SceneRenderer::SceneRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_indexCount(0),
    m_constantBufferPerFrame(nullptr),
    m_constantBufferPerObject(nullptr),
    m_constantBufferNeverChanges(nullptr),
    m_initialized(false)
{
    XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());

    // Initialize device resources asynchronously.
    InitializeInBackground();
}

winrt::Windows::Foundation::IAsyncAction SceneRenderer::InitializeInBackground()
{
    using namespace std::literals::chrono_literals;

    co_await winrt::resume_after(1s);

    m_initialized = true;
}

void SceneRenderer::FinalizeInitialization()
{

}

void SceneRenderer::Render()
{

}

void SceneRenderer::ReleaseResources()
{

}

void SceneRenderer::SetProjMatrix(DirectX::FXMMATRIX projMatrix)
{
    XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void SceneRenderer::SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds)
{

}

void SceneRenderer::SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
{

}