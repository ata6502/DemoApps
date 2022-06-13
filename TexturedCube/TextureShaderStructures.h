#pragma once

struct VertexPositionTexture
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texture;
};

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

struct ConstantBufferNeverChanges
{
    DirectionalLight LightArray[2]; // two directional lights
    MaterialDesc Material;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;
    DirectX::XMFLOAT4X4 TextureTransform;
    DirectX::XMFLOAT3 EyePosition;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
