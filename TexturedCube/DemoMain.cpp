#include "pch.h"

#include "DemoMain.h"
#include "RendererFactory.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace DirectX;

DemoMain::DemoMain() :
    m_rotation(0.0f),
    m_rotationEnabled(false)
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
    
    m_input = std::make_unique<IndependentInput>();

    auto renderer = RendererFactory::CreateRenderer(RendererType::Texture, m_deviceResources);
    m_renderer = std::unique_ptr<RendererBase>(renderer);
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

    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        50.0f);

    m_renderer->SetProjMatrix(projMatrix);
}

void DemoMain::StartRenderLoop()
{
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
    CreateWindowSizeDependentResources(); 
    StartRenderLoop();
}

void DemoMain::Update()
{
    XMVECTOR eye = m_input->GetPosition();
    static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    m_renderer->SetViewMatrix(XMMatrixLookAtLH(eye, at, up), eye, m_timer.GetTotalSeconds());

    if (m_rotationEnabled)
    {
        m_rotation = (m_rotation + m_timer.GetElapsedSeconds());
        if (m_rotation > XM_2PI)
            m_rotation = fmod(m_rotation, XM_2PI);
    }
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

    m_renderer->SetWorldMatrix(XMMatrixRotationY(m_rotation));

    m_renderer->Render();
}

void DemoMain::ReleaseResources()
{
    m_renderer->ReleaseResources();
}

void DemoMain::SetRenderer(int32_t rendererIndex)
{
    if (!m_renderer->IsInitialized())
        return;

    StopRenderLoop();
    ReleaseResources();

    auto renderer = RendererFactory::CreateRenderer((RendererType)rendererIndex, m_deviceResources);
    m_renderer.reset();
    m_renderer.reset(renderer);

    CreateWindowSizeDependentResources();
    StartRenderLoop();
}
