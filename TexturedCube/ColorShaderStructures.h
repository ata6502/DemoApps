#pragma once

// Used to send per-vertex data to the vertex shader.
struct VertexPositionColor
{
	DirectX::XMFLOAT3 Position;
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
