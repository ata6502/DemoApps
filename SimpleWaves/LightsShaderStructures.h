#pragma once

struct VertexPositionNormal
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
