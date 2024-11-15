#pragma once

struct CBufferPerFrame
{
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
};

struct CBufferPerObject
{
    DirectX::XMFLOAT4X4 World;
};
