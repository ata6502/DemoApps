#pragma once

struct VertexPositionColor
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};

struct VertexPositionNormal
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
};

struct VertexPositionTexture
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texture;
};

