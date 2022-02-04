#pragma once

struct DirectionalLightDesc
{
    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;
    DirectX::XMFLOAT3 Direction;
    float Pad;
};

// TODO: Bring other light sources
/*
struct PointLightDesc
{
    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;

    // Packed into 4D vector: (Position, Range)
    DirectX::XMFLOAT3 Position;
    float Range;

    // Packed into 4D vector: (A0, A1, A2, Pad)
    DirectX::XMFLOAT3 Attenuation;
    float Pad;
};

struct SpotLightDesc
{
    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;

    // Packed into 4D vector: (Position, Range)
    DirectX::XMFLOAT3 Position;
    float Range;

    // Packed into 4D vector: (Direction, Spot)
    DirectX::XMFLOAT3 Direction;
    float Spot; // Spot is the exponent s in the equation: kspot(ϕ) = max(cosϕ,0)^s = max(-L•d,0)^s

    // Packed into 4D vector: (Att, Pad)
    DirectX::XMFLOAT3 Attenuation;
    float Pad;
};
*/

struct MaterialDesc
{
    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular; // w = SpecularPower
};

struct ConstantBufferNeverChanges
{
    DirectionalLightDesc DirectionalLight;
};

struct ConstantBufferPerFrame
{
    DirectX::XMFLOAT4X4 ViewProj;

    // TODO: Bring other light sources
    //PointLightDesc PointLight;
    //SpotLightDesc SpotLight;
    DirectX::XMFLOAT3 EyePosition;
};

struct ConstantBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
    MaterialDesc Material;
};
