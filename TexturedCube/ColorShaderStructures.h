#pragma once

struct VertexPosition
{
	DirectX::XMFLOAT3 Position;
};

struct VertexColor
{
    DirectX::XMFLOAT3 Color;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;
    DirectX::XMFLOAT4 Time;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
