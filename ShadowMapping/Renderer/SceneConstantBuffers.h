#pragma once

struct DirectionalLightDesc
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

struct CBufferPerFrame
{
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
    DirectionalLightDesc DirectionalLight;
    DirectX::XMFLOAT3 EyePosition;
    float Pad;
};

struct CBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4X4 WorldInvTranspose;
    MaterialDesc Material;
    DirectX::XMFLOAT4X4 TextureTransform;
    DirectX::XMFLOAT4X4 ShadowTransform; // LT: used with shadows
};
