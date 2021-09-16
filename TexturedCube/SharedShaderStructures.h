#pragma once

// Constant buffer used to send MVP matrices to the vertex shader.
struct ModelViewProjectionConstantBuffer
{
    DirectX::XMFLOAT4X4 Model;
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
};
