#include "pch.h"

#include "MeshFactory.h"

using namespace DirectX;

MeshFactory::MeshFactory(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

void MeshFactory::MakeCube()
{
    const uint16_t CubeVertexCount = 8;
    const uint16_t CubeIndexCount = 36;

    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size();
    info.StartIndexLocation = m_indices.size();
    info.IndexCount = CubeIndexCount;

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + CubeVertexCount);

    m_vertices[i++] = { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

    ASSERT(m_vertices.size() == i);

    i = info.StartIndexLocation;
    m_indices.resize(i + CubeIndexCount);

    m_indices[i++] = 0; m_indices[i++] = 1; m_indices[i++] = 2;
    m_indices[i++] = 1; m_indices[i++] = 3; m_indices[i++] = 2;

    m_indices[i++] = 4; m_indices[i++] = 6; m_indices[i++] = 5;
    m_indices[i++] = 5; m_indices[i++] = 6; m_indices[i++] = 7;

    m_indices[i++] = 0; m_indices[i++] = 5; m_indices[i++] = 1;
    m_indices[i++] = 0; m_indices[i++] = 4; m_indices[i++] = 5;

    m_indices[i++] = 2; m_indices[i++] = 7; m_indices[i++] = 6;
    m_indices[i++] = 2; m_indices[i++] = 3; m_indices[i++] = 7;

    m_indices[i++] = 0; m_indices[i++] = 6; m_indices[i++] = 4;
    m_indices[i++] = 0; m_indices[i++] = 2; m_indices[i++] = 6;

    m_indices[i++] = 1; m_indices[i++] = 7; m_indices[i++] = 3;
    m_indices[i++] = 1; m_indices[i++] = 5; m_indices[i++] = 7;

    ASSERT(m_indices.size() == i);

    m_meshes.push_back(info);
}

void MeshFactory::MakePyramid()
{
    const uint16_t PyramidVertexCount = 5;
    const uint16_t PyramidIndexCount = 18;

    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size();
    info.StartIndexLocation = m_indices.size();
    info.IndexCount = PyramidIndexCount;

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + PyramidVertexCount);

    const float l = 0.5f;
    m_vertices[i++] = { XMFLOAT3(0.0f, l, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };

    ASSERT(m_vertices.size() == i);

    i = info.StartIndexLocation;
    m_indices.resize(i + PyramidIndexCount);

    m_indices[i++] = 0; m_indices[i++] = 1; m_indices[i++] = 2;
    m_indices[i++] = 0; m_indices[i++] = 2; m_indices[i++] = 3;
    m_indices[i++] = 0; m_indices[i++] = 3; m_indices[i++] = 4;
    m_indices[i++] = 0; m_indices[i++] = 4; m_indices[i++] = 1;

    m_indices[i++] = 2; m_indices[i++] = 1; m_indices[i++] = 3;
    m_indices[i++] = 3; m_indices[i++] = 1; m_indices[i++] = 4;

    ASSERT(m_indices.size() == i);

    m_meshes.push_back(info);
}

/*
    Based on [Luna]

    We define a cylinder by specifying:
    - the bottom radius
    - the top radius
    - the cylider height
    - the slice count (a slice is one triangle from the top or the bottom cap)
    - the stack count (a stack is one segment of the cylinder between its "rings")

    The slices and stacks control the triangle density.

    This cylinder has 3 stacks and 4 rings. Its top and bottom radii differ:
       ____
      /    \
     /      \
    /________\

    We break the cylinder into three parts:
    - the side geometry
    - the top cap geometry
    - the bottom cap geometry

    We generate the cylinder centered at the origin, parallel to the y-axis.
    All its vertices lie on the "rings" of the cylinder.

    - ringCount = stackCount + 1
    - each ring has sliceCount unique vertices
    - the difference in radius between consecutive rings: radiusStep = (topRadius – bottomRadius)/stackCount

    If we start at the bottom ring with index 0, then:
    - the radius of the i-th ring is Ri = bottomRadius + i * radiusStep
    - the height of the i-th ring is Hi = -cylinderHeight / 2 + i * stackHeight

    The idea is to iterate over each ring and generate the vertices that lie on that ring.
*/
void MeshFactory::MakeCylinder(float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
{
    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size();
    info.StartIndexLocation = m_indices.size();

    float stackHeight = cylinderHeight / stackCount;

    // Amount to increment radius as we move up each stack from bottom to top.
    float radiusStep = (topRadius - bottomRadius) / stackCount;

    uint32_t ringCount = stackCount + 1;

    // Compute vertices for each stack ring starting at the bottom and moving up.
    for (auto i = 0; i < ringCount; ++i)
    {
        float y = -0.5f * cylinderHeight + i * stackHeight; // the height of the i-th ring
        float r = bottomRadius + i * radiusStep;  // the radius of the i-th ring

        // Vertices of ring.
        // Note that we duplicate the first and last vertex per ring. It has meaning with textures.
        float theta = XM_2PI / sliceCount; // an "top" angle of a single slice triangle
        for (auto j = 0; j <= sliceCount; ++j)
        {
            float x = r * cosf(j * theta);
            float z = r * sinf(j * theta);

            VertexPositionColor v;
            v.Position = XMFLOAT3(x, y, z);

            // Alternate color for each stack.
            if ((i + 1) % 2)
                v.Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
            else
                v.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0);

            m_vertices.push_back(v);
        }
    }

    // Add one because we duplicate the first and last vertex per ring
    // since the texture coordinates are different.
    auto n = sliceCount + 1; // the number of vertices per ring

    // Compute indices for each stack.
    for (auto i = 0; i < stackCount; ++i)
    {
        for (auto j = 0; j < sliceCount; ++j)
        {
            // Each quad is composed of two triangles: ABC and ACD

            auto A = i * n + j;
            auto B = (i + 1) * n + j;
            auto C = (i + 1) * n + j + 1;
            auto D = i * n + j + 1;

            m_indices.push_back(A);
            m_indices.push_back(B);
            m_indices.push_back(C);

            m_indices.push_back(A);
            m_indices.push_back(C);
            m_indices.push_back(D);
        }
    }

    BuildCylinderTopCap(info.BaseVertexLocation, bottomRadius, topRadius, cylinderHeight, sliceCount, stackCount);
    BuildCylinderBottomCap(info.BaseVertexLocation, bottomRadius, topRadius, cylinderHeight, sliceCount, stackCount);

    info.IndexCount = m_indices.size() - info.StartIndexLocation;

    m_meshes.push_back(info);
}

void MeshFactory::BuildCylinderTopCap(uint32_t baseVertexLocation, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
{
    uint32_t baseIndex = (uint32_t)m_vertices.size() - baseVertexLocation;

    float y = 0.5f * cylinderHeight; // the height of the top cap
    float theta = XM_2PI / sliceCount;

    VertexPositionColor v;
    v.Color = XMFLOAT4(0.3f, 0.3f, 0.0f, 1.0f);

    // Duplicate top cap ring vertices because the texture coordinates and normals differ (TODO: texture and normals)
    for (auto i = 0; i <= sliceCount; ++i)
    {
        float x = topRadius * cosf(i * theta);
        float z = topRadius * sinf(i * theta);

        v.Position = XMFLOAT3(x, y, z);
        m_vertices.push_back(v);

    }

    // Cap center vertex.
    v.Position = XMFLOAT3(0.0f, y, 0.0f);
    v.Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_vertices.push_back(v);

    // Index of the center vertex.
    uint32_t centerIndex = (uint32_t)m_vertices.size() - baseVertexLocation - 1;

    for (auto i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(centerIndex);
        m_indices.push_back(baseIndex + i + 1);
        m_indices.push_back(baseIndex + i);
    }
}

void MeshFactory::BuildCylinderBottomCap(uint32_t baseVertexLocation, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
{
    uint32_t baseIndex = (uint32_t)m_vertices.size() - baseVertexLocation;

    float y = -0.5f * cylinderHeight; // the height of the bottom cap
    float theta = XM_2PI / sliceCount;

    VertexPositionColor v;
    v.Color = XMFLOAT4(0.3f, 0.3f, 0.0f, 1.0f);

    // Duplicate bottom cap ring vertices.
    for (auto i = 0; i <= sliceCount; ++i)
    {
        float x = bottomRadius * cosf(i * theta);
        float z = bottomRadius * sinf(i * theta);

        v.Position = XMFLOAT3(x, y, z);
        m_vertices.push_back(v);
    }

    // Cap center vertex.
    v.Position = XMFLOAT3(0.0f, y, 0.0f);
    v.Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_vertices.push_back(v);

    // Index of the center vertex.
    uint32_t centerIndex = (uint32_t)m_vertices.size() - baseVertexLocation - 1;

    for (auto i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(centerIndex);
        m_indices.push_back(baseIndex + i);
        m_indices.push_back(baseIndex + i + 1);
    }
}

void MeshFactory::Build()
{
    // TODO: Create const vertex buffers.
    
    // Create vertex buffer and load data.
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = m_vertices.data();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(m_vertices.size() * sizeof(VertexPositionColor), D3D11_BIND_VERTEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            m_vertexBuffer.put()));

    // Create index buffer and load indices to the buffer.
    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = m_indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indices.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_indexBuffer.put()));
}

void MeshFactory::Set()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Each vertex is one instance of the VertexPositionColor struct.
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    // Each index is one 32-bit unsigned integer.
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
}

void MeshFactory::Draw(int index)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    auto info = m_meshes[index];

    context->DrawIndexed(info.IndexCount, info.StartIndexLocation, info.BaseVertexLocation);
}

void MeshFactory::Release()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}
