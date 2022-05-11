#include "pch.h"
#include "LightsController.h"

using namespace DirectX;

void LightsController::CreateLights()
{
    // Create the point light source.
    m_pointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_pointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_pointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_pointLight.Position = XMFLOAT3(0.0f, 0.0f, 0.0f); // we change the light's position dynamically
    m_pointLight.Range = 25.0f;
    m_pointLight.Attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);

    // Create the spot light source.
    m_spotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_spotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
    m_spotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_spotLight.Position = XMFLOAT3(0.0f, 0.0f, 0.0f); // we change the light's position dynamically
    m_spotLight.Range = 10000.0f;
    m_spotLight.Direction = XMFLOAT3(0.0f, 0.0f, 0.0f); // we change the light's direction dynamically
    m_spotLight.Spot = 128.0f; // we change the light's cone angle dynamically
    m_spotLight.Attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
}

void LightsController::UpdatePointLight(DirectX::FXMVECTOR const& position)
{
    XMStoreFloat3(&m_pointLight.Position, position);
}

void LightsController::UpdateSpotLight(DirectX::FXMVECTOR const& position, DirectX::FXMVECTOR const& direction)
{
    XMStoreFloat3(&m_spotLight.Position, position);
    XMStoreFloat3(&m_spotLight.Direction, direction);
}

void LightsController::SetSpotlightConeHalfAngle(int halfAngleIndex)
{
    auto halfAnglePowers = std::vector<int>{ 256, 128, 64, 32, 16, 8, 4, 2, 1 };
    ASSERT(halfAngleIndex < halfAnglePowers.size());

    m_spotLight.Spot = halfAnglePowers[halfAngleIndex];
}