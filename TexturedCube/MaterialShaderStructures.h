#pragma once

struct DirectionalLight
{
    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;
    DirectX::XMFLOAT3 Direction;
    float Pad;
};

struct MaterialDesc
{
    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular; // w = SpecularPower
};

// Used to send per-vertex data to the vertex shader.
struct VertexPositionNormal
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
};

struct ConstantBufferNeverChanges
{
    DirectionalLight Light;
    MaterialDesc Material;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;
    DirectX::XMFLOAT3 EyePosition;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
