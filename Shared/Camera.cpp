#include "pch.h"

#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
    m_lookingAtPosition = XMFLOAT3(0.0f, -0.1f, 0.0f);
}

DirectX::XMMATRIX Camera::GetProjMatrix(winrt::Windows::Foundation::Size const& outputSize)
{
    static float fovAngleY = 70.0f * XM_PI / 180.0f;

    float aspectRatio = outputSize.Width / outputSize.Height;

    XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        300.0f);

    return projMatrix;
}

DirectX::XMMATRIX Camera::GetViewMatrix(DirectX::FXMVECTOR eye)
{
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMVECTOR at = XMLoadFloat3(&m_lookingAtPosition);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, at, up);

    return viewMatrix;
}

DirectX::XMVECTOR Camera::GetLookingAtPosition() const
{
    return XMLoadFloat3(&m_lookingAtPosition);
}