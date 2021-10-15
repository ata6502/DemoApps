#pragma once

#include "ColorShaderStructures.h" // TODO: move it to .cpp when pImpl used
#include "DeviceResources.h"

class GridMesh
{
public:
    GridMesh(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void Create(float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth);
    void SetBuffers();
    void Draw();
    void ReleaseResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    winrt::com_ptr<ID3D11Buffer>            m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;

    uint32_t                                m_indexCount;
};

