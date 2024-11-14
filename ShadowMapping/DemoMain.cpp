#include "pch.h"
#include "DemoMain.h"

using namespace Concurrency;
using namespace DirectX;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System::Threading;

DemoMain::DemoMain() :
    m_hasFocus(false),
    m_rotation(0.f),
    m_rotationEnabled(true)
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    m_renderer = std::make_unique<MainRenderer>(m_deviceResources);

    m_input = std::make_unique<IndependentInput>();
    m_input->SetInputRadius(30.f);

    Initialize();
}

DemoMain::~DemoMain()
{
    StopRenderLoop();

    // Deregister device notification
    m_deviceResources->RegisterDeviceNotify(nullptr);
    m_input->StopProcessEvents();
}

winrt::fire_and_forget DemoMain::Initialize()
{
    auto lifetime = get_strong();

    co_await m_renderer->CreateDeviceDependentResourcesAsync();

    critical_section::scoped_lock lock(m_criticalSection);
    m_renderer->FinalizeCreateDeviceResources();
    CreateWindowSizeDependentResources();
}

void DemoMain::StartRenderLoop()
{
    // If the animation render loop is already running then do not start another thread.
    if (m_renderLoopWorker != nullptr && m_renderLoopWorker.Status() == AsyncStatus::Started)
        return;

    m_timer.ResetElapsedTime();

    // Create a task that will run on a background thread.
    auto workItemHandler = ([this](IAsyncAction const& action)
        {
            // Calculate the updated frame and render once per vertical blanking interval.
            while (action.Status() == AsyncStatus::Started)
            {
                // Ensure smooth transition between renderers by not proceeding 
                // to Preset if the renderer is not initialized. This is only useful
                // if we switch between multiple renderers.
                if (!m_renderer->IsInitialized())
                    continue;

                critical_section::scoped_lock lock(m_criticalSection);

                Update();
                m_renderer->Render();
                m_deviceResources->Present();

                if (!m_hasFocus)
                {
                    // The app is in an inactive state. We can stop rendering. This optimizes 
                    // power consumption and allows the framework to become more quiescent.
                    break;
                }
            }
        });

    // Run task on a dedicated high priority background thread.
    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DemoMain::StopRenderLoop()
{
    if (m_renderLoopWorker != nullptr)
        m_renderLoopWorker.Cancel();
}

void DemoMain::FocusChanged(bool hasFocus)
{
    m_hasFocus = hasFocus;

    if (m_hasFocus)
        StartRenderLoop();
    else
        StopRenderLoop();
}

void DemoMain::DpiChanged(float logicalDpi)
{
    critical_section::scoped_lock lock(m_criticalSection);
    m_deviceResources->SetDpi(logicalDpi);
    CreateWindowSizeDependentResources();
}

void DemoMain::OrientationChanged(winrt::Windows::Graphics::Display::DisplayOrientations currentOrientation)
{
    critical_section::scoped_lock lock(m_criticalSection);
    m_deviceResources->SetCurrentOrientation(currentOrientation);
    CreateWindowSizeDependentResources();
}

void DemoMain::ValidateDevice()
{
    critical_section::scoped_lock lock(m_criticalSection);
    m_deviceResources->ValidateDevice();
}

void DemoMain::SwapChainPanelSizeChanged(winrt::Windows::Foundation::Size const& logicalSize)
{
    critical_section::scoped_lock lock(m_criticalSection);

    m_deviceResources->SetLogicalSize(logicalSize);
    CreateWindowSizeDependentResources();
}

void DemoMain::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
    critical_section::scoped_lock lock(m_criticalSection);
    m_deviceResources->SetCompositionScale(compositionScaleX, compositionScaleY);
    CreateWindowSizeDependentResources();
}

void DemoMain::Suspend()
{
    critical_section::scoped_lock lock(m_criticalSection);

    // Stop rendering when the app is suspended.
    StopRenderLoop();
    m_deviceResources->Trim();
}

void DemoMain::Resume()
{
    StartRenderLoop();
}

void DemoMain::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel)
{
    m_deviceResources->SetSwapChainPanel(panel);
    m_input->Initialize(panel);
}

void DemoMain::OnDeviceLost()
{
    StopRenderLoop();
    m_renderer->ReleaseDeviceDependentResources();
}

void DemoMain::OnDeviceRestored()
{
    auto lifetime = get_strong();

    Initialize();
    StartRenderLoop();
}

void DemoMain::CreateWindowSizeDependentResources()
{
    if (!m_renderer->IsInitialized())
        return;

    winrt::Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 0.25f * XM_PI;

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f);

    XMMATRIX orientationMatrix = m_deviceResources->GetOrientationTransform3D();

    m_renderer->CreateWindowSizeDependentResources(
        XMMatrixMultiply(
            orientationMatrix,
            projectionMatrix
        )
    );
}

void DemoMain::Update()
{
    if (!m_renderer->IsInitialized())
        return;

    m_timer.Tick([&]()
    {
        float elapsedSeconds{ static_cast<float>(m_timer.GetElapsedSeconds()) };

        if (m_rotationEnabled)
        {
            // The value 0.6 controls the rotation speed.
            m_rotation = m_rotation + 0.6f * elapsedSeconds;
            if (m_rotation > XM_2PI)
                m_rotation = fmod(m_rotation, XM_2PI);
            // m_renderer->ExecuteCommand(SimpleCommand("SetRotation", m_rotation)); // TODO: set rotation in renderers
        }

        //m_renderer->ExecuteCommand(SimpleCommand("ElapsedSeconds", elapsedSeconds)); // TODO: set elapsed time in renderers

        static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
        static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
        XMVECTOR eye = m_input->GetPosition();
        XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, at, up);

        m_renderer->Update(viewMatrix, eye);
    });
}
