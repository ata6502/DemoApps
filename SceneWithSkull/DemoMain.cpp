#include "pch.h"

#include "DemoMain.h"
#include "RendererFactory.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace DirectX;

DemoMain::DemoMain() :
    m_scissorTestEnabled(false)
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    m_input = std::make_unique<IndependentInput>();
    m_camera = std::make_unique<Camera>();

    auto renderer = RendererFactory::CreateRenderer(RendererType::Color, m_deviceResources); // TODO: set TextureRenderer as default
    m_renderer = std::unique_ptr<RendererBase>(renderer);
    m_input->SetRadius(m_renderer->GetDistanceToCamera());
    m_input->SetPitch(m_renderer->GetCameraPitch());

    m_timer.Reset();
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
    m_renderer->SetOutputSize(outputSize);
}

void DemoMain::StartRenderLoop()
{
    using namespace std::literals::chrono_literals;

    // Do not start another thread if the render loop is already running.
    if ((m_renderLoopWorker != nullptr && m_renderLoopWorker.Status() == AsyncStatus::Started))
        return;

    m_timer.Start();

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

                Update();
                Render();

                m_deviceResources->Present();

                m_timer.Update();
            }
        });

    // Run the task on a dedicated high priority background thread.
    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DemoMain::StopRenderLoop()
{
    m_timer.Stop();
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
    CreateWindowSizeDependentResources();
    StartRenderLoop();
}

void DemoMain::Update()
{
    XMVECTOR eye = m_input->GetPosition();
    auto viewMatrix = m_camera->GetViewMatrix(eye);

    m_renderer->Update(viewMatrix, eye, m_timer.GetElapsedSeconds());
}

/// <summary>
/// Renders the current frame.
/// </summary>
void DemoMain::Render()
{
    // Don't render anything before the first Update and until the renderer is initialized.
    if (fabs(m_timer.GetTotalSeconds()) < std::numeric_limits<float>::epsilon())
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Reset render targets to the screen.
    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

    // Clear the back buffer and depth stencil view.
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);

    m_renderer->Render();
}

void DemoMain::ReleaseResources()
{
    m_renderer->ReleaseResources();
}

void DemoMain::ToggleScissorTest(float leftRightMarginPercent, float topBottomMarginPercent)
{ 
    m_scissorTestEnabled = !m_scissorTestEnabled;

    SetScissorTestLeftRightMargin(leftRightMarginPercent);
    SetScissorTestTopBottomMargin(topBottomMarginPercent);
    m_renderer->EnableScissorTest(m_scissorTestEnabled);
}

void DemoMain::SetScissorTestLeftRightMargin(float marginPercent)
{
    m_renderer->SetScissorTestLeftRightMargin(marginPercent);
}

void DemoMain::SetScissorTestTopBottomMargin(float marginPercent)
{
    m_renderer->SetScissorTestTopBottomMargin(marginPercent);
}

void DemoMain::SetRenderer(int32_t rendererIndex)
{
    if (!m_renderer->IsInitialized())
        return;

    StopRenderLoop();

    critical_section::scoped_lock lock(m_criticalSection);

    ReleaseResources();

    auto renderer = RendererFactory::CreateRenderer((RendererType)rendererIndex, m_deviceResources);
    m_renderer.reset();
    m_renderer.reset(renderer);
    m_input->SetRadius(m_renderer->GetDistanceToCamera());
    m_input->SetPitch(m_renderer->GetCameraPitch());

    CreateWindowSizeDependentResources();
    StartRenderLoop();
}
