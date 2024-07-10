#pragma once

#include "DeviceResources.h"
#include "VertexStructures.h"

class ColorMeshGenerator
{
public:
    ColorMeshGenerator(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void CreateCube(std::string const& name);
    void CreatePyramid(std::string const& name);
    void CreateCylinder(std::string const& name, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount);
    void CreateSphere(std::string const& name, float radius, uint32_t sliceCount, uint32_t stackCount);
    void CreateGeosphere(std::string const& name, float radius, uint16_t subdivisionCount);
    void CreateGrid(std::string const& name, float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth);

    void CreateBuffers();
    void SetBuffers();
    void DrawMesh(std::string const& name);
    void Clear();

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
    std::map<std::string, MeshInfo>         m_meshes;

    void BuildCylinderTopCap(uint32_t baseVertexLocation, float topRadius, float cylinderHeight, uint32_t sliceCount);
    void BuildCylinderBottomCap(uint32_t baseVertexLocation, float bottomRadius, float cylinderHeight, uint32_t sliceCount);
    void Subdivide(std::vector<VertexPositionColor>& vertices, std::vector<uint32_t>& indices);
    void CopyIndices(std::vector<uint32_t> const& indices, uint32_t startIndexLocation, size_t indexCount);
};

