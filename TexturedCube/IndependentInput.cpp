#include "pch.h"
#include "IndependentInput.h"

using namespace DirectX;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;

const float IndependentInput::MIN_INPUT_RADIUS = 1.8f;
const float IndependentInput::MAX_INPUT_RADIUS = 8.0f;
const float IndependentInput::DEFAULT_INPUT_RADIUS = 2.2f;
const float IndependentInput::DEFAULT_INPUT_YAW = 1.5f * XM_PI;
const float IndependentInput::DEFAULT_INPUT_PITCH = 0.37f * XM_PI;

IndependentInput::IndependentInput() :
    m_radius(DEFAULT_INPUT_RADIUS),
    m_yaw(DEFAULT_INPUT_YAW),
    m_pitch(DEFAULT_INPUT_PITCH),
    m_mouseInUse(false), 
    m_leftButtonPressed(false), 
    m_rightButtonPressed(false),
    m_mouseLastPosition(Point(0.f,0.f))
{
} 

void IndependentInput::Initialize(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel)
{
    // Set up the independent input source to run in a background thread and hook up to a few pointer events.
    auto workItemHandler = ([this, panel](IAsyncAction const& action)
    {
        // The CoreIndependentInputSource mathod raises pointer events for the specified device types on whichever thread 
        // it's created on. Using the CreateCoreIndependentInputSource method, apps can process input and render to a SwapChainPanel 
        // on one or more background threads. This enables high performance input and rendering independent of the XAML UI thread.

        auto coreInput = panel.CreateCoreIndependentInputSource(Windows::UI::Core::CoreInputDeviceTypes::Mouse);

        // Register for pointer events, which will be raised on the background thread.
        coreInput.PointerPressed({ this, &IndependentInput::OnPointerPressed });
        coreInput.PointerMoved({ this, &IndependentInput::OnPointerMoved });
        coreInput.PointerReleased({ this, &IndependentInput::OnPointerReleased });

        // Begin processing input messages as they're delivered.
        coreInput.Dispatcher().ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessUntilQuit);
    });

    // Run task on a dedicated high priority background thread.
    m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void IndependentInput::OnPointerPressed([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Core::PointerEventArgs const& args)
{
    if (m_mouseInUse == false &&
        (args.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::LeftButtonPressed ||
         args.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::RightButtonPressed))
    {
        // Store active pointer ID: only one contact can be manipulating at a time.
        m_activePointerId = args.CurrentPoint().PointerId();
        m_mouseLastPosition = args.CurrentPoint().Position();

        m_leftButtonPressed = (args.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::LeftButtonPressed);
        m_rightButtonPressed = (args.CurrentPoint().Properties().PointerUpdateKind() == PointerUpdateKind::RightButtonPressed);

        m_mouseInUse = true;
    }
}

void IndependentInput::OnPointerMoved([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Core::PointerEventArgs const& args)
{
    if (m_mouseInUse && args.CurrentPoint().PointerId() == m_activePointerId)
    {
        Point currPosition = args.CurrentPoint().Position();

        if (m_leftButtonPressed)
        {
            // Make each pixel correspond to a quarter of a degree.
            float dx = XMConvertToRadians(0.25f * (currPosition.X - m_mouseLastPosition.X));
            float dy = XMConvertToRadians(0.25f * (currPosition.Y - m_mouseLastPosition.Y));

            m_yaw += dx;
            m_pitch += dy;

            // Restrict the pitch to be between 0.1f and Pi-0.1f
            m_pitch = std::clamp(m_pitch, 0.1f, XM_PI - 0.1f);
        }
        else if (m_rightButtonPressed)
        {
            float dx = 0.02f * (currPosition.X - m_mouseLastPosition.X);
            float dy = 0.02f * (currPosition.Y - m_mouseLastPosition.Y);

            m_radius += dx - dy;

            // Restrict the radius.
            m_radius = std::clamp(m_radius, MIN_INPUT_RADIUS, MAX_INPUT_RADIUS);
        }

        m_mouseLastPosition = currPosition;
    }
}

void IndependentInput::OnPointerReleased([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender, [[maybe_unused]] winrt::Windows::UI::Core::PointerEventArgs const& args)
{
    m_mouseInUse = false;
    m_leftButtonPressed = false;
    m_rightButtonPressed = false;
}
