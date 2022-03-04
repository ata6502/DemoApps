#pragma once

class MathHelper
{
public:
    // Returns random float in [0, 1).
    static float MathHelper::RandF();

    // Returns random float in [a, b).
    static float MathHelper::RandF(float a, float b);

    // [Luna] Returns the polar angle of the point (x,y) in [0, 2*PI).
    static float MathHelper::AngleFromXY(float x, float y);
    
private:    
    static const float Pi;
};
