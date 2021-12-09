#pragma once

#include "DeviceResources.h"

class GridMesh
{
public:
    GridMesh(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void Create(float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth, std::function<float(float, float)> heightFunction, DirectX::XMFLOAT4 color, DirectX::XMFLOAT4 altColor);

    void SetBuffers();
    void Draw();
    void ReleaseResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    winrt::com_ptr<ID3D11Buffer>            m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;

    uint32_t                                m_indexCount;
};

