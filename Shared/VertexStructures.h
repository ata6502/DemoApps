#pragma once

struct VertexPosition
{
    DirectX::XMFLOAT3 Position;
};

struct VertexPositionColor
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Color;
};

struct VertexPositionNormal
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
};

struct VertexPositionTexture
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 Texture;
};

struct VertexPositionNormalTexture
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texture;
};
