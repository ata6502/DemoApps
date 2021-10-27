#pragma once

#include "ColorShaderStructures.h"
#include "DeviceResources.h"

class MeshFactory
{
public:
    MeshFactory(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void AddCube();
    void AddPyramid();

    void Build();
    void Set();
    void Draw(int index);
    void Release();

private:
    struct MeshInfo
    {
        uint32_t IndexCount;
        uint32_t StartIndexLocation;
        uint32_t BaseVertexLocation;
    };

    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    winrt::com_ptr<ID3D11Buffer>            m_vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>            m_indexBuffer;

    std::vector<VertexPositionColor>        m_vertices;
    std::vector<uint32_t>                   m_indices;
    std::vector<MeshInfo>                   m_meshes;
};

