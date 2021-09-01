#pragma once

#include "pch.h"

class Timer
{
public:
    Timer() :
        m_active(false)
    {
        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency))
        {
            winrt::throw_hresult(E_FAIL);
        }
        m_secondsPerCount = 1.0f / static_cast<float>(frequency.QuadPart);

        Reset();
    }

    void Start()
    {
        LARGE_INTEGER startTime;
        if (!QueryPerformanceCounter(&startTime))
        {
            winrt::throw_hresult(E_FAIL);
        }
        if (!m_active)
        {
            // Accumulate the time elapsed between stop and start pairs.
            m_pausedTime.QuadPart += (startTime.QuadPart - m_stopTime.QuadPart);

            m_previousTime = startTime;
            m_stopTime.QuadPart = 0;
            m_active = true;
        }
    }

    void Stop()
    {
        if (m_active)
        {
            // Set the stop time to the time of the last update.
            m_stopTime = m_currentTime;
            m_active = false;
        }
    }

    void Update()
    {
        if (!m_active)
        {
            m_deltaTime = 0.0;
            return;
        }

        LARGE_INTEGER currentTime;
        if (!QueryPerformanceCounter(&currentTime))
        {
            winrt::throw_hresult(E_FAIL);
        }
        m_currentTime = currentTime;

        m_deltaTime = (m_currentTime.QuadPart - m_previousTime.QuadPart) * m_secondsPerCount;
        m_previousTime = m_currentTime;

        if (m_deltaTime < 0.0)
        {
            m_deltaTime = 0.0;
        }
    }

    void Reset()
    {
        LARGE_INTEGER currentTime;
        if (!QueryPerformanceCounter(&currentTime))
        {
            winrt::throw_hresult(E_FAIL);
        }
        m_baseTime = currentTime;
        m_previousTime = currentTime;
        m_stopTime = currentTime;
        m_currentTime = currentTime;
        m_pausedTime.QuadPart = 0;
        m_active = false;
    }

    // Returns the total time elapsed in seconds since Reset() was called, not counting any
    // time when the clock is stopped.
    float GetTotalSeconds()
    {
        if (m_active)
        {
            // The distance m_currentTime - m_baseTime includes paused time,
            // which we do not want to count. To correct this, we can subtract
            // the paused time from m_currentTime:
            return static_cast<float>(((m_currentTime.QuadPart - m_pausedTime.QuadPart) - m_baseTime.QuadPart) * m_secondsPerCount);
        }
        else
        {
            // The clock is currently not running so don't count the time since
            // the clock was stopped
            return static_cast<float>(((m_stopTime.QuadPart - m_pausedTime.QuadPart) - m_baseTime.QuadPart) * m_secondsPerCount);
        }
    }

    // Returns elapsed time in seconds between the last two updates.
    float GetElapsedSeconds()
    {
        return m_deltaTime;
    }

private:
    float m_secondsPerCount;  // 1.0 / Frequency
    float m_deltaTime;

    LARGE_INTEGER m_baseTime;
    LARGE_INTEGER m_pausedTime;
    LARGE_INTEGER m_stopTime;
    LARGE_INTEGER m_previousTime;
    LARGE_INTEGER m_currentTime;

    bool m_active;
};
