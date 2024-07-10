#pragma once

class Boid
{
public:
    Boid(DirectX::FXMVECTOR position, DirectX::FXMVECTOR velocity, float maxVelocity);
    void Update(DirectX::FXMVECTOR velocityDelta);
    DirectX::XMVECTOR GetPosition() const;
    DirectX::XMVECTOR GetVelocity() const;
    DirectX::XMMATRIX GetWorldMatrix() const;
    void SetPosition(DirectX::FXMVECTOR position);
    void SetVelocity(DirectX::FXMVECTOR velocity);

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_velocity;
    float m_maxVelocity;
};

