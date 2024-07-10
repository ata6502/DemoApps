#include "pch.h"

#include "Boid.h"

using namespace DirectX;

Boid::Boid(DirectX::FXMVECTOR position, DirectX::FXMVECTOR velocity, float maxVelocity) :
    m_maxVelocity(maxVelocity)
{
    XMStoreFloat3(&m_position, position);
    XMStoreFloat3(&m_velocity, velocity);
}

void Boid::Update(DirectX::FXMVECTOR velocityDelta)
{
    XMVECTOR newVelocity = XMVectorAdd(XMLoadFloat3(&m_velocity), velocityDelta);

    // Limit the boid's speed i.e., limit the magnitude of the boid's velocity.
    float speed = XMVectorGetX(XMVector3Length(newVelocity));
    if (speed > m_maxVelocity)
        newVelocity = (newVelocity / speed) * m_maxVelocity;
    XMStoreFloat3(&m_velocity, newVelocity);

    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat3(&m_position), newVelocity);
    XMStoreFloat3(&m_position, newPosition);
}

DirectX::XMVECTOR Boid::GetPosition() const
{
    return XMLoadFloat3(&m_position);
}

DirectX::XMVECTOR Boid::GetVelocity() const
{
    return XMLoadFloat3(&m_velocity);
}

DirectX::XMMATRIX Boid::GetWorldMatrix() const
{
    return XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
}

void Boid::SetPosition(DirectX::FXMVECTOR position)
{
    XMStoreFloat3(&m_position, position);
}

void Boid::SetVelocity(DirectX::FXMVECTOR velocity)
{
    XMStoreFloat3(&m_velocity, velocity);
}
