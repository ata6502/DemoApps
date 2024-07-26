#pragma once

#include "DeviceResources.h"

class SkySphere
{
public:
    SkySphere(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);
    void SetBuffers();
    void DrawSphere();
    void ReleaseDeviceDependentResources();

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    winrt::com_ptr<ID3D11Buffer>            m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;
    uint32_t                                m_indexCount;
};

