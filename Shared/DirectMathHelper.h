#pragma once

// Computes a face normal of a triangle P1P2P3.
inline DirectX::XMVECTOR ComputeNormal(const DirectX::FXMVECTOR& p0, const DirectX::FXMVECTOR& p1, const DirectX::FXMVECTOR& p2)
{
    using namespace DirectX;

    XMVECTOR u = p1 - p0;
    XMVECTOR v = p2 - p0;
    return XMVector3Normalize(XMVector3Cross(u, v));
}

// Converts XMVECTOR to XMFLOAT3
inline DirectX::XMFLOAT3 VectorToFloat3(const DirectX::FXMVECTOR& v)
{
    using namespace DirectX;

    XMFLOAT3 f;
    XMStoreFloat3(&f, v);
    return f;
}