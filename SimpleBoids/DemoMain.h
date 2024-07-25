#pragma once

#include "BoidParameter.h"
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
    float GetBoidParameter(BoidParameter parameter) const { return m_swarm->GetBoidParameter(parameter); }
    bool GetIsVisualRangeEnabled() const { return m_swarm->IsVisualRangeEnabled(); }

    // Setters
    void SetBoidShape(int32_t boidShapeIndex) { m_boidShapeIndex = boidShapeIndex; }
    void SetBoidParameter(BoidParameter parameter, float value) { m_swarm->SetBoidParameter(parameter, value); }
    void SetIsVisualRangeEnabled(bool enabled) { m_swarm->IsVisualRangeEnabled(enabled); }

    // App-specific methods.
    void RestartSimulation();
    void AddBoids();
    void RemoveBoids();

    // IDeviceNotify
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

private:
    // Boid constants.
    static const int INITIAL_BOID_COUNT = 200;
    static const int BOID_COUNT_TO_ADD = 20;
    static const int BOID_COUNT_TO_REMOVE = 10;
    static const float BOID_RADIUS;
    static const int BOID_SUBDIVISION_COUNT = 3;
    static const float BOID_MIN_DISTANCE;           // the minimum distance between boids
    static const float BOID_MATCHING_FACTOR;        // adjustment of average velocity as % (matching factor)
    static const float MAX_BOID_SPEED;              // the max length of the velocity vector
    static const float BOID_AVOID_FACTOR;           // scales the vectors that repel boids
    static const float BOID_TURN_FACTOR;            // encourages boids to fly in a particular direction
    static const float BOID_VISUAL_RANGE;           // used in calculating boid's velocity while taking into account only boids in a certain range
    static const float BOID_MOVE_TO_CENTER_FACTOR;  // determines how to move a boid towards the center (percentage)

    // Boundary constants.
    static const float BOX_EDGE_LENGTH;
    static const float BOX_EDGE_THICKNESS; 

    // Input constants.
    static const float INPUT_RADIUS;                // the distance from the eye
    static const float INPUT_YAW;                   // "horizontal" angle
    static const float INPUT_PITCH;                 // "vertical" angle
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
    DirectX::XMFLOAT4X4                         m_waterTextureTransform;

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

