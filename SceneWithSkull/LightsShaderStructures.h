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

struct ConstantBufferNeverChanges
{
    DirectionalLightDesc DirectionalLightArray[3];
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;

    // The eye position is needed to calculate specular 
    // highlights which depend on the eye position.
    DirectX::XMFLOAT3 EyePosition;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4X4 WorldInvTranspose;
    MaterialDesc Material;
};
