#pragma once

class Camera
{
public:
    Camera();

    DirectX::XMMATRIX GetProjMatrix(winrt::Windows::Foundation::Size const& outputSize);
    DirectX::XMMATRIX GetViewMatrix(DirectX::FXMVECTOR eye);
    DirectX::XMVECTOR GetLookingAtPosition() const;

private:
    DirectX::XMFLOAT3 m_lookingAtPosition;
};

