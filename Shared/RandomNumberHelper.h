#pragma once

#include <random> // mt19937, uniform_real_distribution

class RandomNumberHelper
{
public:
    RandomNumberHelper();
    float GetFloat(float min, float max);

private:
    std::unique_ptr<std::mt19937> m_randomNumberEngine;
    std::uniform_real_distribution<float> m_distribution;
};

