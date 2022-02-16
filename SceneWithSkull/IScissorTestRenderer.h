#pragma once

class IScissorTestRenderer
{
public:
    virtual void EnableScissorTest(bool enabled) = 0;
    virtual void SetScissorTestLeftRightMargin(float marginPercent) = 0;
    virtual void SetScissorTestTopBottomMargin(float marginPercent) = 0;
};