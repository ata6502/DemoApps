#include "pch.h"

#include "DemoMain.h"

using namespace Concurrency;
using namespace DirectX;

const float DemoMain::BOID_RADIUS = 1.5f;
const float DemoMain::BOID_MIN_DISTANCE = 1.0f;
const float DemoMain::BOID_MATCHING_FACTOR = 0.2f;
const float DemoMain::MAX_BOID_SPEED = 0.7f;
const float DemoMain::BOID_AVOID_FACTOR = 0.3f;
const float DemoMain::BOID_TURN_FACTOR = 0.5f;
const float DemoMain::BOID_VISUAL_RANGE = 3.0f;
const float DemoMain::BOID_MOVE_TO_CENTER_FACTOR = 0.01f;

const float DemoMain::BOX_EDGE_LENGTH = 45.0f;
const float DemoMain::BOX_EDGE_THICKNESS = 2.f;

const float DemoMain::INPUT_RADIUS = 250.f;
const float DemoMain::INPUT_YAW = -0.5f * DirectX::XM_PI;
const float DemoMain::INPUT_PITCH = 0.35f * DirectX::XM_PI;
const float DemoMain::INPUT_STEP = 0.15f;

DemoMain::DemoMain() :
    m_hasFocus(false),
    m_boidShapeIndex(0)
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
    m_sceneRenderer = std::make_unique<SceneRenderer>(m_deviceResources);
    XMStoreFloat4x4(&m_waterTextureTransform, XMMatrixIdentity());

    // Configure input.
    m_input = std::make_unique<IndependentInput>();
    m_input->SetInputRadius(INPUT_RADIUS);
    m_input->SetInputYaw(INPUT_YAW);
    m_input->SetInputPitch(INPUT_PITCH);
    m_input->SetInputStep(INPUT_STEP);

    m_swarm = std::make_unique<Swarm>(
        BOID_RADIUS, 
        BOID_MIN_DISTANCE, 
        BOID_MATCHING_FACTOR, 
        MAX_BOID_SPEED,
        BOID_AVOID_FACTOR,
        BOID_TURN_FACTOR,
        BOID_VISUAL_RANGE,
        BOID_MOVE_TO_CENTER_FACTOR,
        BOX_EDGE_LENGTH);
    m_swarm->AddBoids(INITIAL_BOID_COUNT);

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

    co_await m_sceneRenderer->CreateDeviceResourcesAsync();
    m_sceneRenderer->CreateSphereMesh("sphereMesh", BOID_RADIUS, BOID_SUBDIVISION_COUNT);
    m_sceneRenderer->CreateCylinderMesh("coneMesh", 2.f, 0.f, 5.f, 12, 4);
    m_sceneRenderer->CreateCubeMesh("cube");
    m_sceneRenderer->CreateGridMesh("water", 800.0f, 800.0f, 80, 80); // TODO: adjust the water parameters
    m_sceneRenderer->FinalizeCreateMeshes();

    // Create materials.
    MaterialDesc boidMaterial;
    ZeroMemory(&boidMaterial, sizeof(boidMaterial));
    boidMaterial.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    boidMaterial.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    boidMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f); // w = SpecularPower
    m_sceneRenderer->AddMaterial("boid", boidMaterial);

    MaterialDesc cubeMaterial;
    ZeroMemory(&cubeMaterial, sizeof(cubeMaterial));
    cubeMaterial.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    cubeMaterial.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
    cubeMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f); // w = SpecularPower
    m_sceneRenderer->AddMaterial("cube", cubeMaterial);

    MaterialDesc waterMaterial;
    waterMaterial.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
    waterMaterial.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 0.5f); // 0.5f is a semi-transparent diffuse component
    waterMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f); // w = SpecularPower
    m_sceneRenderer->AddMaterial("water", waterMaterial);

    // Create textures.
    m_sceneRenderer->AddTexture("boid", L"Assets\\Textures\\marble.dds");
    m_sceneRenderer->AddTexture("cube", L"Assets\\Textures\\wood.dds");
    m_sceneRenderer->AddTexture("water", L"Assets\\Textures\\water.dds");

    // The subsequent methods use DeviceContext. We need to sync the threads.
    critical_section::scoped_lock lock(m_criticalSection);

    // Create the light 
    DirectionalLightDesc light;
    ZeroMemory(&light, sizeof(light));
    light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    light.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    light.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    light.Direction = XMFLOAT3(0.51451f, -0.51451f, 0.68601f);
    m_sceneRenderer->SetLight(light);

    m_sceneRenderer->FinalizeCreateDeviceResources();

    CreateWindowSizeDependentResources();
}

void DemoMain::StartRenderLoop()
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::System::Threading;

    // Do not start another thread if the render loop is already running.
    if (m_renderLoopWorker != nullptr && m_renderLoopWorker.Status() == AsyncStatus::Started)
        return;

    m_timer.ResetElapsedTime();

    // Create a task that will run on a background thread.
    auto workItemHandler = ([this](IAsyncAction const& action)
        {
            // Calculate the updated frame and render once per vertical blanking interval.
            while (action.Status() == AsyncStatus::Started)
            {
                critical_section::scoped_lock lock(m_criticalSection);

                Update();
                m_sceneRenderer->PrepareRender();
                DrawScene();
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
    // DeviceResources::SetLogicalSize calls DeviceResources::CreateWindowSizeDependentResources which
    // uses device context. We need to synchronize threads so only one accessed the device context.
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
    m_sceneRenderer->ReleaseDeviceDependentResources();
}

void DemoMain::OnDeviceRestored()
{
    auto lifetime = get_strong();

    Initialize();
    StartRenderLoop();
}

void DemoMain::CreateWindowSizeDependentResources()
{
    m_sceneRenderer->CreateWindowSizeDependentResources();
}

void DemoMain::Update()
{
    m_timer.Tick([&]()
        {
            static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
            static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

            XMVECTOR eye = m_input->GetPosition();
            XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, at, up);
            m_sceneRenderer->Update(eye, viewMatrix);

            float timeDelta{ static_cast<float>(m_timer.GetElapsedSeconds()) };

            m_swarm->Update(timeDelta);

            //
            // Animate water texture coordinates.
            //
            static XMFLOAT2 waterTextureOffset = XMFLOAT2(0, 0);

            // Tile the water texture.
            XMMATRIX wavesScale = XMMatrixScaling(12.0f, 12.0f, 0.0f);

            // Scroll the water texture over the water geometry as a function of time.
            waterTextureOffset.y += 0.02f * timeDelta;
            waterTextureOffset.x += 0.05f * timeDelta;
            XMMATRIX wavesOffset = XMMatrixTranslation(waterTextureOffset.x, waterTextureOffset.y, 0.0f);

            // Combine scale and translation.
            XMStoreFloat4x4(&m_waterTextureTransform, wavesScale * wavesOffset);
        });
}

void DemoMain::DrawScene()
{
    // Draw the boundary for boids as a wired box.
    m_sceneRenderer->SetMaterial("cube");
    m_sceneRenderer->SetTexture("cube");

    // Vertical edges along the y-axis.
    XMMATRIX scaling;
    m_sceneRenderer->SetTextureTransform(XMMatrixTranspose(XMMatrixRotationZ(-XM_PIDIV2)));
    scaling = XMMatrixScaling(BOX_EDGE_THICKNESS, 2 * BOX_EDGE_LENGTH + BOX_EDGE_THICKNESS, BOX_EDGE_THICKNESS);
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(-BOX_EDGE_LENGTH, 0.f, -BOX_EDGE_LENGTH)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(BOX_EDGE_LENGTH, 0.f, -BOX_EDGE_LENGTH)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(-BOX_EDGE_LENGTH, 0.f, BOX_EDGE_LENGTH)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(BOX_EDGE_LENGTH, 0.f, BOX_EDGE_LENGTH)));

    // Horizontal edges along the x-axis.
    m_sceneRenderer->SetTextureTransform(XMMatrixIdentity());
    scaling = XMMatrixScaling(2 * BOX_EDGE_LENGTH, BOX_EDGE_THICKNESS, BOX_EDGE_THICKNESS);
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(0.f, -BOX_EDGE_LENGTH, -BOX_EDGE_LENGTH)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(0.f, BOX_EDGE_LENGTH, -BOX_EDGE_LENGTH)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(0.f, -BOX_EDGE_LENGTH, BOX_EDGE_LENGTH)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(0.f, BOX_EDGE_LENGTH, BOX_EDGE_LENGTH)));

    // Horizontal edges along the z-axis.
    m_sceneRenderer->SetTextureTransform(XMMatrixIdentity());
    scaling = XMMatrixScaling(BOX_EDGE_THICKNESS, BOX_EDGE_THICKNESS, 2 * BOX_EDGE_LENGTH);
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(-BOX_EDGE_LENGTH, -BOX_EDGE_LENGTH, 0.f)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(-BOX_EDGE_LENGTH, BOX_EDGE_LENGTH, 0.f)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(BOX_EDGE_LENGTH, BOX_EDGE_LENGTH, 0.f)));
    DrawCube(XMMatrixMultiply(scaling, XMMatrixTranslation(BOX_EDGE_LENGTH, -BOX_EDGE_LENGTH, 0.f)));

    // Draw the swarm of boids.
    m_sceneRenderer->SetMaterial("boid");
    m_sceneRenderer->SetTexture("boid");
    m_sceneRenderer->SetTextureTransform(XMMatrixIdentity()); // no texture transform for boids

    std::string meshName;
    switch (m_boidShapeIndex)
    {
    case 0:
        meshName = "sphereMesh";
        break;
    case 1:
        meshName = "coneMesh";
        break;
    default:
        meshName = "sphereMesh";
        break;
    }

    m_swarm->Iterate([this, &meshName](XMMATRIX worldMatrix)
        {
            m_sceneRenderer->SetWorldMatrix(worldMatrix);
            m_sceneRenderer->RenderMesh(meshName);
        });

    // Draw water.
    m_sceneRenderer->SetMaterial("water");
    m_sceneRenderer->SetTexture("water");
    m_sceneRenderer->SetTextureTransform(XMLoadFloat4x4(&m_waterTextureTransform));
    m_sceneRenderer->SetWorldMatrix(XMMatrixTranslation(0.f, -BOX_EDGE_LENGTH - 0.5f * BOX_EDGE_THICKNESS - 0.2f, 0.f));
    m_sceneRenderer->SetTransparentBlendState();
    m_sceneRenderer->RenderMesh("water");
    m_sceneRenderer->ClearTransparentBlendState();
}

void DemoMain::DrawCube(DirectX::FXMMATRIX worldMatrix)
{
    m_sceneRenderer->SetWorldMatrix(worldMatrix);
    m_sceneRenderer->RenderMesh("cube");
}

void DemoMain::RestartSimulation()
{
    m_swarm->ResetBoids();
}

void DemoMain::AddBoids()
{
    m_swarm->AddBoids(BOID_COUNT_TO_ADD);
}

void DemoMain::RemoveBoids()
{
    m_swarm->RemoveBoids(BOID_COUNT_TO_REMOVE);
}
