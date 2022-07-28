#pragma once

#include "LightsShaderStructures.h"

class MaterialController
{
public:
    void CreateMaterials();

    void SetTerrainSpecularComponent(int specularComponent);
    void SetWaveSpecularComponent(int specularComponent);

    MaterialDesc GetTerrainMaterial() const { return m_terrainMaterial; }
    MaterialDesc GetWaveMaterial() const { return m_waveMaterial; }
    MaterialDesc GetBoxMaterial() const { return m_boxMaterial; }

private:
    int DEFAULT_TERRAIN_SPECULAR_COMPONENT = 2;
    int DEFAULT_WAVE_SPECULAR_COMPONENT = 4;

    MaterialDesc m_terrainMaterial;
    MaterialDesc m_waveMaterial;
    MaterialDesc m_boxMaterial;

    float ConvertSpecularComponentToSpecularPowerValue(int specularComponent) const;
};

