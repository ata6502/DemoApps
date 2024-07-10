#pragma once

#include "DeviceResources.h"
#include "IndependentInput.h"
#include "Renderer.h"
#include "StepTimer.h"
#include "Swarm.h"

class DemoMain : public winrt::implements<DemoMain, winrt::Windows::Foundation::IInspectable>, public DX::IDeviceNotify
{
public:
    DemoMain();
    ~DemoMain();

    void FocusChanged(bool hasFocus);
    void DpiChanged(float logicalDpi);
    void OrientationChanged(winrt::Windows::Graphics::Display::DisplayOrientations currentOrientation);
    void ValidateDevice();

    void SwapChainPanelSizeChanged(winrt::Windows::Foundation::Size const& logicalSize);
    void SetCompositionScale(float compositionScaleX, float compositionScaleY);

    void Suspend();
    void Resume();

    void SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel);

    // Getters
    size_t GetSwarmSize() const { return m_swarm->Size(); }
    float GetBoidMinDistance() const { return m_swarm->GetBoidMinDistance(); }
    float GetBoidMatchingFactor() const { return m_swarm->GetBoidMatchingFactor(); }

    // Setters
    void SetBoidShape(int32_t boidShapeIndex) { m_boidShapeIndex = boidShapeIndex; }
    void SetBoidMinDistance(float boidMinDistance) { m_swarm->SetBoidMinDistance(boidMinDistance); }
    void SetBoidMatchingFactor(float boidMatchingFactor) { m_swarm->SetBoidMatchingFactor(boidMatchingFactor); }

    // App-specific methods.
    void RestartSimulation();
    void AddBoids();
    void RemoveBoids();

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

private:
    // Boid constants.
    static const int BOID_COUNT_TO_ADD = 20;
    static const int BOID_COUNT_TO_REMOVE = 10;
    static const float BOID_RADIUS;
    static const int BOID_SUBDIVISION_COUNT = 3;
    static const float BOID_MIN_DISTANCE;       // the minimum distance between boids
    static const float BOID_MATCHING_FACTOR;    // adjustment of average velocity as % (boid matching factor)


    // Input constants.
    static const float INPUT_RADIUS; // the distance from the eye
    static const float INPUT_YAW;    // "horizontal" angle
    static const float INPUT_PITCH;  // "vertical" angle
    static const float INPUT_STEP;

    std::shared_ptr<DX::DeviceResources>        m_deviceResources;
    std::unique_ptr<Renderer>                   m_renderer;
    DX::StepTimer                               m_timer;
    Concurrency::critical_section               m_criticalSection;
    winrt::Windows::Foundation::IAsyncAction    m_renderLoopWorker;
    std::unique_ptr<IndependentInput>           m_input;
    bool                                        m_hasFocus;
    std::unique_ptr<Swarm>                      m_swarm;
    int32_t                                     m_boidShapeIndex;

    // Private helper methods.
    void StartRenderLoop();
    void StopRenderLoop();
    winrt::fire_and_forget Initialize();
    void CreateWindowSizeDependentResources();
    void Update();

    // App-specific methods.
    void DrawCube(DirectX::FXMMATRIX worldMatrix);
    void DrawScene();
};

