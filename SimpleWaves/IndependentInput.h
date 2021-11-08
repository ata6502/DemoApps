#pragma once

class IndependentInput
{
public:
    IndependentInput();

    void Initialize(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel);
    void StopProcessEvents();

    // Convert spherical coordinates to Cartesian coordinates.
    DirectX::XMVECTOR GetPosition() const
    {
        DirectX::XMVECTOR pos =
            DirectX::XMVectorSet(
                m_radius * sinf(m_pitch) * cosf(m_yaw),
                m_radius * cosf(m_pitch),
                m_radius * sinf(m_pitch) * sinf(m_yaw),
                0.0f);
        return pos;
    }

private:
    static const float MIN_INPUT_RADIUS;
    static const float MAX_INPUT_RADIUS;
    static const float DEFAULT_INPUT_RADIUS;
    static const float DEFAULT_INPUT_YAW;
    static const float DEFAULT_INPUT_PITCH;

    // Independent input handlers.
    void OnPointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Core::PointerEventArgs const& args);
    void OnPointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Core::PointerEventArgs const& args);
    void OnPointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Core::PointerEventArgs const& args);

    // Gesture recognizer handlers.
    void OnManipulationUpdated(winrt::Windows::UI::Input::GestureRecognizer const& recognizer, winrt::Windows::UI::Input::ManipulationUpdatedEventArgs const& args);

    // Track independent input on a background worker thread.
    winrt::Windows::Foundation::IAsyncAction m_inputLoopWorker;
    winrt::Windows::UI::Core::CoreIndependentInputSource m_coreInput{ nullptr };
    uint32_t m_activePointerId;
    Concurrency::critical_section m_criticalSection;

    winrt::Windows::UI::Input::GestureRecognizer m_gestureRecognizer{ nullptr };

    float                               m_radius;
    float                               m_yaw; // "horizontal" angle
    float                               m_pitch; // "vertical" angle
    bool                                m_mouseInUse;
    bool                                m_leftButtonPressed;
    bool                                m_rightButtonPressed;
    winrt::Windows::Foundation::Point   m_mouseLastPosition;
};

