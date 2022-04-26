#pragma once

#include "LightsShaderStructures.h"

class LightsController
{
public:
    void CreateLights();

    void UpdatePointLight(DirectX::FXMVECTOR const& position);
    void UpdateSpotLight(DirectX::FXMVECTOR const& position, DirectX::FXMVECTOR const& direction);
    void SetSpotlightConeHalfAngle(int halfAnglePower);

    PointLightDesc GetPointLight() const { return m_pointLight; }
    SpotLightDesc GetSpotLight() const { return m_spotLight; }

private:
    // The description of a point light source. We need to change its position every frame.
    PointLightDesc m_pointLight;

    // The description of a spot light source. We need to change its position and direction every frame.
    SpotLightDesc m_spotLight;
};

