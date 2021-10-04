#include "pch.h"

#include "Camera.h"

using namespace DirectX;

DirectX::XMMATRIX Camera::GetProjMatrix(winrt::Windows::Foundation::Size const& outputSize)
{
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f);

    return projMatrix;
}

DirectX::XMMATRIX Camera::GetViewMatrix(DirectX::FXMVECTOR eye)
{
    static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, at, up);

    return viewMatrix;
}

