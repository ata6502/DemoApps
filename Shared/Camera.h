#pragma once

class Camera
{
public:
    Camera();

    DirectX::XMMATRIX GetProjMatrix(winrt::Windows::Foundation::Size const& outputSize);
    DirectX::XMMATRIX GetViewMatrix(DirectX::FXMVECTOR eye);
    DirectX::XMVECTOR GetLookAtPosition() const;

private:
    static const float FOV_ANGLE_Y;
    static const float NEAR_PLANE;
    static const float FAR_PLANE;
    static const DirectX::XMVECTORF32 LOOK_AT_POS_DEFAULT;
    static const DirectX::XMVECTORF32 LOOK_AT_UP;

    DirectX::XMFLOAT3 m_lookAtPosition;
};
