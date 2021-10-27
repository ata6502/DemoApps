#pragma once

struct VertexPositionColor
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
