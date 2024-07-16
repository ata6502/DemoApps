#include "pch.h"

#include "Boid.h"

using namespace DirectX;

Boid::Boid(DirectX::FXMVECTOR position, DirectX::FXMVECTOR velocity, float maxSpeed) :
    m_maxSpeed(maxSpeed)
{
    XMStoreFloat3(&m_position, position);
    XMStoreFloat3(&m_velocity, velocity);
}

void Boid::Update(DirectX::FXMVECTOR velocityDelta)
{
    XMVECTOR newVelocity = XMVectorAdd(XMLoadFloat3(&m_velocity), velocityDelta);

    // Limit the boid's speed i.e., limit the magnitude of the boid's velocity.
    float speed = XMVectorGetX(XMVector3Length(newVelocity));
    if (speed > m_maxSpeed)
        newVelocity = (newVelocity / speed) * m_maxSpeed;
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
    // Adjust the boid orientation.

    // Compute the angle between the current velocity and the initial boid orientation v0.
    XMVECTOR v0 = XMVectorSet(0.f, 0.f, 1.f, 1.f);
    XMVECTOR v1 = XMVector3Normalize(XMLoadFloat3(&m_velocity));
    float angle = acos(XMVectorGetX(XMVector3Dot(v0, v1)));

    // Compute rotation axis.
    XMVECTOR rotAxis = XMVector3Normalize(XMVector3Cross(v0, v1));

    // Compute rotation matrix.
    XMMATRIX rotMatrix = XMMatrixRotationAxis(rotAxis, angle);

    return XMMatrixRotationX(XM_PIDIV2) * // initial boid orientation
        rotMatrix * 
        XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
}

void Boid::SetPosition(DirectX::FXMVECTOR position)
{
    XMStoreFloat3(&m_position, position);
}

void Boid::SetVelocity(DirectX::FXMVECTOR velocity)
{
    XMStoreFloat3(&m_velocity, velocity);
}

void Boid::SetMaxSpeed(float maxSpeed)
{
    m_maxSpeed = maxSpeed;
}
