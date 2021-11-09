#pragma once

#include "ColorShaderStructures.h"
#include "DeviceResources.h"

class MeshGenerator
{
public:
    MeshGenerator(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void CreateCube();
    void CreatePyramid();
    void CreateCylinder(float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount);
    void CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);
    void CreateGeosphere(float radius, uint16_t numSubdivisions);
    void CreateGrid(float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth);

    void CreateBuffers();
    void SetBuffers();
    void DrawMesh(int index);
    void ReleaseBuffers();

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

    void BuildCylinderTopCap(uint32_t baseVertexLocation, float topRadius, float cylinderHeight, uint32_t sliceCount);
    void BuildCylinderBottomCap(uint32_t baseVertexLocation, float bottomRadius, float cylinderHeight, uint32_t sliceCount);
    void Subdivide(std::vector<VertexPositionColor>& vertices, std::vector<uint32_t>& indices);
    void CopyIndices(std::vector<uint32_t> const& indices, uint32_t startIndexLocation, size_t indexCount);
};

