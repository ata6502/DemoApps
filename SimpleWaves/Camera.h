#pragma once

class Camera
{
public:
    DirectX::XMMATRIX GetProjMatrix(winrt::Windows::Foundation::Size const& outputSize);
    DirectX::XMMATRIX GetViewMatrix(DirectX::FXMVECTOR eye);
};

