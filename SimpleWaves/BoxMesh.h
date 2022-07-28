#pragma once

#include "DeviceResources.h"

class BoxMesh
{
public:
    BoxMesh(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void CreateWithTexture();

    void SetBuffers(unsigned int stride);
    void Draw();
    void ReleaseResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    winrt::com_ptr<ID3D11Buffer>            m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;

    uint32_t                                m_indexCount;
};

