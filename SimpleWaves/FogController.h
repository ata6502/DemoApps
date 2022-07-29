#pragma once

class FogController
{
public:
    FogController();

    void SetFogStart(float fogStart);
    void SetFogRange(float fogRange);

    float GetFogStart() const { return m_fogStart; }
    float GetFogRange() const { return m_fogRange; }
    DirectX::XMFLOAT4 GetFogColor() { return m_fogColor; }

private:
    static const float DEFAULT_FOG_START;
    static const float DEFAULT_FOG_RANGE;

    DirectX::XMFLOAT4 m_fogColor;
    float m_fogStart; // the fog start distance from the camera
    float m_fogRange; // the range from the fog start distance until the fog completely hides any objects
};

