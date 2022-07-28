#include "pch.h"
#include "MaterialController.h"

using namespace DirectX;

void MaterialController::CreateMaterials()
{
    float specularPower = ConvertSpecularComponentToSpecularPowerValue(DEFAULT_TERRAIN_SPECULAR_COMPONENT);
    m_terrainMaterial.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
    m_terrainMaterial.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
    m_terrainMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, specularPower);

    specularPower = ConvertSpecularComponentToSpecularPowerValue(DEFAULT_WAVE_SPECULAR_COMPONENT);
    m_waveMaterial.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
    m_waveMaterial.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 0.5f); // the diffuse component of the wave material has alpha < 1.0f, i.e. it is transparent
    m_waveMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, specularPower);

    // TODO: Control spec component for box.
    specularPower = ConvertSpecularComponentToSpecularPowerValue(DEFAULT_WAVE_SPECULAR_COMPONENT);
    m_boxMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_boxMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_boxMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, specularPower);
}

/// <summary>
/// [Luna] Ex.2 p.278 Modify the lighting demo by changing the specular power material component, 
/// which controls the "shininess" of the surface. 
/// </summary>
/// <param name="specularPower">A number between 1 and 7 representig the specular power material component</param>
void MaterialController::SetTerrainSpecularComponent(int specularComponent)
{
    float specularPower = ConvertSpecularComponentToSpecularPowerValue(specularComponent);
    m_terrainMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, specularPower);
}

void MaterialController::SetWaveSpecularComponent(int specularComponent)
{
    float specularPower = ConvertSpecularComponentToSpecularPowerValue(specularComponent);
    m_waveMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, specularPower);
}

float MaterialController::ConvertSpecularComponentToSpecularPowerValue(int specularComponent) const
{
    switch (specularComponent)
    {
    case 1:
        return 8.0f;
    case 2:
        return 16.0f;
    case 3:
        return 32.0f;
    case 4:
        return 64.0f;
    case 5:
        return 128.0f;
    case 6:
        return 256.0f;
    case 7:
        return 512.0f;
    default:
        return 16.0f;
    }
}

