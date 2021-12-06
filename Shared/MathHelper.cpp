#include "pch.h"
#include "MathHelper.h"

// Returns random float in [0, 1).
float MathHelper::RandF()
{
    return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
float MathHelper::RandF(float a, float b)
{
    return a + RandF() * (b - a);
}
