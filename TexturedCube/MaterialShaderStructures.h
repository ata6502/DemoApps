#pragma once

#include "SharedShaderStructures.h"

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

struct LightMaterialEyeConstantBuffer
{
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
