#include "pch.h"

#include "Camera.h"

using namespace DirectX;

const float Camera::FOV_ANGLE_Y = 70.0f * XM_PI / 180.0f;
const float Camera::NEAR_PLANE = 0.01f;
const float Camera::FAR_PLANE = 200.0f;
const XMVECTORF32 Camera::LOOK_AT_POS_DEFAULT = { 0.0f, -0.1f, 0.0f, 0.0f };
const XMVECTORF32 Camera::LOOK_AT_UP = { 0.0f, 1.0f, 0.0f, 0.0f };

Camera::Camera()
{
    XMStoreFloat3(&m_lookAtPosition, LOOK_AT_POS_DEFAULT);
}

DirectX::XMMATRIX Camera::GetProjMatrix(winrt::Windows::Foundation::Size const& outputSize)
{
    float aspectRatio = outputSize.Width / outputSize.Height;

    XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(
        FOV_ANGLE_Y,
        aspectRatio,
        NEAR_PLANE,
        FAR_PLANE);

    return projMatrix;
}

DirectX::XMMATRIX Camera::GetViewMatrix(DirectX::FXMVECTOR eye)
{
    XMVECTOR at = XMLoadFloat3(&m_lookAtPosition);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, at, LOOK_AT_UP);

    return viewMatrix;
}

DirectX::XMVECTOR Camera::GetLookAtPosition() const
{
    return XMLoadFloat3(&m_lookAtPosition);
}