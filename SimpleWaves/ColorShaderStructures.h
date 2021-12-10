#pragma once

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
