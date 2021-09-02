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

// Constant buffer used to send MVP matrices to the vertex shader.
struct ModelViewProjectionConstantBuffer
{
    DirectX::XMFLOAT4X4 Model;
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
    DirectionalLight Light;
    MaterialDesc Material;
    DirectX::XMFLOAT3 EyePosition;
};

// Used to send per-vertex data to the vertex shader.
struct VertexPositionNormal
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
};
