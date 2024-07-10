#include "pch.h"
#include "RandomNumberHelper.h"

#include <chrono>

RandomNumberHelper::RandomNumberHelper()
{
    auto seed = (unsigned int)std::chrono::system_clock::now()
        .time_since_epoch().count();
    m_randomNumberEngine = std::make_unique<std::mt19937>(seed);
}

float RandomNumberHelper::GetFloat(float min, float max)
{
    return m_distribution(*m_randomNumberEngine.get()) * (max - min) + min;
}
