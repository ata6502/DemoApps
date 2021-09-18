#pragma once

struct ConstantBufferNeverChanges
{
    DirectionalLight Light;
    MaterialDesc Material;
};

struct ConstantBufferOnResize
{
    DirectX::XMFLOAT4X4 Projection;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT3 EyePosition;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 Model;
};

