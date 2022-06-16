#include "pch.h"

#include "DemoMain.h"
#include "RendererFactory.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace DirectX;

DemoMain::DemoMain() :
    m_renderer(nullptr),
    m_currentFillMode("solid")
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    m_input = std::make_unique<IndependentInput>();
    m_camera = std::make_unique<Camera>();
    m_materialController = std::make_shared<MaterialController>();
    m_lightsController = std::make_shared<LightsController>();

    auto renderer = RendererFactory::CreateRenderer(RendererType::Wave, m_deviceResources, m_materialController, m_lightsController);
    m_renderer = std::unique_ptr<RendererBase>(renderer);

    m_shaderController = std::make_unique<ShaderController>(m_deviceResources);
    m_rasterizerStateManager = std::make_unique<RasterizerStateManager>(m_deviceResources);

    // Create rasterizer states.
    m_rasterizerStateManager->AddRasterizerState("solid", RasterizerState::FillMode::Solid, RasterizerState::CullMode::CullBack, RasterizerState::WindingOrder::Clockwise);
    m_rasterizerStateManager->AddRasterizerState("wireframe", RasterizerState::FillMode::Wireframe, RasterizerState::CullMode::CullBack, RasterizerState::WindingOrder::Clockwise);
}

DemoMain::~DemoMain()
{
    // Deregister device notification.
    m_deviceResources->RegisterDeviceNotify(nullptr);

    // Stop rendering and processing events on destruction.
    StopRenderLoop();
    m_input->StopProcessEvents();
}

void DemoMain::CreateWindowSizeDependentResources()
{
    Size outputSize = m_deviceResources->GetOutputSize();
    auto projMatrix = m_camera->GetProjMatrix(outputSize);
    m_renderer->SetProjMatrix(projMatrix);
}

void DemoMain::StartRenderLoop()
{
    using namespace std::literals::chrono_literals;

    // Do not start another thread if the render loop is already running.
    if ((m_renderLoopWorker != nullptr && m_renderLoopWorker.Status() == AsyncStatus::Started))
        return;

    // Create a task that will be run on a background thread.
    auto workItemHandler = ([this](IAsyncAction const& action)
        {
            m_renderer->FinalizeInitialization();

            // Calculate the updated frame and render once per vertical blanking interval.
            while (action.Status() == AsyncStatus::Started)
            {
                critical_section::scoped_lock lock(m_criticalSection);

                // Check if the renderer is initialized before calling any
                // update or render methods.
                if (!m_renderer->IsInitialized())
                    continue;

                m_timer.Tick([&]()
                {
                    Update();
                });
                Render();

                m_deviceResources->Present();
            }
        });

    // Run the task on a dedicated high priority background thread.
    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DemoMain::StopRenderLoop()
{
    m_renderLoopWorker.Cancel();
}

void DemoMain::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel)
{
    m_deviceResources->SetSwapChainPanel(panel);
    m_input->Initialize(panel);
}

void DemoMain::SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize)
{
    m_deviceResources->SetLogicalSize(logicalSize);
}

void DemoMain::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
    m_deviceResources->SetCompositionScale(compositionScaleX, compositionScaleY);
}

void DemoMain::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}

void DemoMain::Suspend()
{
    StopRenderLoop();
    m_deviceResources->Trim();
}

void DemoMain::Resume()
{
    StartRenderLoop();
}

void DemoMain::OnDeviceLost()
{
    StopRenderLoop();
    ReleaseResources();
}

void DemoMain::OnDeviceRestored()
{
    m_renderer->InitializeInBackground();
    m_shaderController->InitializeInBackground();
    CreateWindowSizeDependentResources();
    StartRenderLoop();
}

void DemoMain::Update()
{
    XMVECTOR eye = m_input->GetPosition();
    auto viewMatrix = m_camera->GetViewMatrix(eye);
    XMVECTOR at = m_camera->GetLookAtPosition();

    m_renderer->SetViewMatrix(viewMatrix, eye, m_timer.GetTotalSeconds());
    m_renderer->Update(m_timer.GetTotalSeconds(), m_timer.GetElapsedSeconds(), eye, at);
}

/// <summary>
/// Renders the current frame.
/// </summary>
void DemoMain::Render()
{
    // Don't render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Reset render targets to the screen.
    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

    // Clear the back buffer and depth stencil view.
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);

    // Set the rasterizer state.
    m_rasterizerStateManager->SetRasterizerState(m_currentFillMode);

    m_shaderController->Render();
    m_renderer->Render();
}

void DemoMain::ReleaseResources()
{
    m_renderer->ReleaseResources();
    m_shaderController->ReleaseResources();
    m_rasterizerStateManager->ReleaseResources();
}

void DemoMain::SetRenderer(int32_t rendererIndex)
{
    if (!m_renderer->IsInitialized())
        return;

    StopRenderLoop();

    critical_section::scoped_lock lock(m_criticalSection);

    m_renderer->ReleaseResources();

    auto renderer = RendererFactory::CreateRenderer((RendererType)rendererIndex, m_deviceResources, m_materialController, m_lightsController);
    m_renderer.reset();
    m_renderer.reset(renderer);

    CreateWindowSizeDependentResources();
    StartRenderLoop();
}

void DemoMain::SetShader(ShaderType shaderType)
{
    m_shaderController->SetShader(shaderType);
}

void DemoMain::SetWireframeFillMode()
{
    m_currentFillMode = "wireframe";
}

void DemoMain::SetSolidFillMode()
{
    m_currentFillMode = "solid";
}

bool DemoMain::IsToonShaderSupported() const
{
    return m_renderer->IsToonShaderSupported();
}

bool DemoMain::AreLightParametersSupported() const
{
    return m_renderer->AreLightParametersSupported();
}

void DemoMain::SetSpecularComponent(int specularComponent)
{
    m_materialController->SetTerrainSpecularComponent(specularComponent);
    m_materialController->SetWaveSpecularComponent(specularComponent);
}

void DemoMain::SetSpotlightConeHalfAngle(int halfAngleIndex)
{
    m_lightsController->SetSpotlightConeHalfAngle(halfAngleIndex);
}
