#include "pch.h"

#include "GridMesh.h"

using namespace DirectX;

GridMesh::GridMesh(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_indexCount(0)
{
}

/// <summary>
/// Builds a grid mesh in the xz-plane procedurally.
/// </summary>
/// <param name="gridWidth">Grid width. It determines the relative size of the grid.</param>
/// <param name="gridDepth">Grid depth. It determines the relative size of the grid.</param>
/// <param name="quadCountHoriz">The number of quads in the grid in the horizontal dimension (x-axis)</param>
/// <param name="quadCountDepth">The number of quads in the grid in the depth dimension (z-axis)</param>
void GridMesh::Create(float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth)
{
    auto colorBlue = XMFLOAT3(0.0f, 0.0f, 1.0f);
    auto dx = gridWidth / quadCountHoriz; // the quad spacing along the x-axis 
    auto dz = gridDepth / quadCountDepth; // the quad spacing along the z-axis
    auto halfWidth = 0.5 * gridWidth;
    auto halfDepth = 0.5 * gridDepth;

    // The grid is built from an M x N matrix of vertices. 
    auto m = quadCountDepth + 1;
    auto n = quadCountHoriz + 1;

    // Create vertices.
    auto vertexCount = m * n;
    std::vector<VertexPositionColor> vertices(vertexCount);

    // An example of a 2 x 4 grid mesh:
    // - quadCountHoriz = 4
    // - quadCountDepth = 2
    // - quadCount = 4 * 2 = 8
    // - triangleCount = quadCount * 2 = 16
    // - m = 3 (the depth number of vertices)
    // - n = 5 (the horizontal number of vertices)
    // - vertexCount = 15
    // 
    //  0--1--2--3--4
    //  |\ |\ |\ |\ | 
    //  | \| \| \| \|
    //  5--6--7--8--9
    //  |\ |\ |\ |\ | 
    //  | \| \| \| \|
    // 10-11-12-13-14

    // Compute vertex positions by starting at the upper-left corner of the grid. 
    // Then, incrementally compute the vertex coordinates row-by-row. 
    auto z = halfDepth;
    for (auto i = 0; i < m; ++i)
    {
        auto x = -halfWidth;

        for (auto j = 0; j < n; ++j)
        {
            auto k = i * n + j;
            vertices[k].Position = XMFLOAT3(x, 0, z);
            vertices[k].Color = colorBlue;

            x += dx;
        };

        z -= dz;
    }

    // Create vertex buffer and load data.
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = vertices.data();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(vertices.size() * sizeof(VertexPositionColor), D3D11_BIND_VERTEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            m_vertexBuffer.put()));

    // Create indices: 
    // - each quad has two triangles
    // - each triangle has three vertices
    // - each quad is duplicated for the top and the bottom face of the grid
    m_indexCount = 2 * quadCountHoriz * quadCountDepth * 2 * 3;

    std::vector<uint16_t> indices(m_indexCount);
    size_t k = 0;

    for (auto i = 0; i < quadCountDepth; ++i)
    {
        for (auto j = 0; j < quadCountHoriz; ++j)
        {
            // Compute four indices of a single quad composed of two triangles: ABD and ADC. 
            // The bottom face of the grid has the same indices but in opposite order.
            //
            //     a----b
            //     |\   |
            //     | \  |
            //     |  \ |
            //     |   \|
            //     c----d
            //
            uint16_t a = j + i * n;
            uint16_t b = j + 1 + i * n;
            uint16_t c = j + (i + 1) * n;
            uint16_t d = j + 1 + (i + 1) * n;

            // top face
            indices[k] = a;
            indices[k + 1] = b;
            indices[k + 2] = d;
            indices[k + 3] = a;
            indices[k + 4] = d;
            indices[k + 5] = c;
            k += 6;

            // bottom face
            indices[k] = a;
            indices[k + 1] = d;
            indices[k + 2] = b;
            indices[k + 3] = a;
            indices[k + 4] = c;
            indices[k + 5] = d;
            k += 6;
        };
    }

    ASSERT(k == m_indexCount);

    // Create index buffer and load indices to the buffer.
    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(indices.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_indexBuffer.put()));
}

/// <summary>
/// Builds a two-color grid mesh in the xz-plane procedurally.
/// </summary>
/// <param name="gridWidth">Grid width. It determines the relative size of the grid.</param>
/// <param name="gridDepth">Grid depth. It determines the relative size of the grid.</param>
/// <param name="quadCountHoriz">The number of quads in the grid in the horizontal dimension (x-axis)</param>
/// <param name="quadCountDepth">The number of quads in the grid in the depth dimension (z-axis)</param>
/// <param name="color">The color of the grid.</param>
/// <param name="altColor">The color of the pattern on the grid.</param>
void GridMesh::Create(float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth, XMFLOAT3 color, XMFLOAT3 altColor)
{
    auto dx = gridWidth / quadCountHoriz; // the quad spacing along the x-axis 
    auto dz = gridDepth / quadCountDepth; // the quad spacing along the z-axis
    auto halfWidth = 0.5 * gridWidth;
    auto halfDepth = 0.5 * gridDepth;

    // The grid is built from an M x N matrix of vertices. 
    auto m = quadCountDepth + 1;
    auto n = quadCountHoriz + 1;

    // Create vertices.
    auto vertexCount = m * n;
    std::vector<VertexPositionColor> vertices(vertexCount);

    // An example of a grid mesh:
    // - quadCountHoriz = 4
    // - quadCountDepth = 2
    // - quadCount = 4 * 2 = 8
    // - triangleCount = quadCount * 2 = 16
    // - m = 3 (the depth number of vertices)
    // - n = 5 (the horizontal number of vertices)
    // - vertexCount = 15
    // 
    //  0--1--2--3--4
    //  |\ | /|\ | /| 
    //  | \|/ | \|/ |
    //  5--6--7--8--9
    //  | /|\ | /|\ | 
    //  |/ | \|/ | \|
    // 10-11-12-13-14

    //  O--O--O--O--O     O represents one color
    //  | /|\ | /|\ |     X represents another color
    //  |/ | \|/ | \|
    //  X--O--X--O--X
    //  | \| /|\ | /|
    //  | \|/ | \|/ |
    //  O--O--O--O--O
    //  | /|\ | /|\ |
    //  |/ | \|/ | \|
    //  X--O--X--O--X
    //  | \| /|\ | /|
    //  | \|/ | \|/ |
    //  O--O--O--O--O

    // Compute vertex positions by starting at the upper-left corner of the grid. 
    // Then, incrementally compute the vertex coordinates row-by-row. 
    auto z = halfDepth;
    for (auto i = 0; i < m; ++i)
    {
        auto x = -halfWidth;

        for (auto j = 0; j < n; ++j)
        {
            auto k = i * n + j;
            vertices[k].Position = XMFLOAT3(x, 0, z);

            // Set the same color for vertices in even rows.
            if (i % 2 == 0)
                vertices[k].Color = color;
            // Flip colors of vertices in odd rows.
            else if (j % 2 == 0)
                vertices[k].Color = color;
            else
                vertices[k].Color = altColor;

            x += dx;
        };

        z -= dz;
    }

    // Create vertex buffer and load data.
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = vertices.data();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(vertices.size() * sizeof(VertexPositionColor), D3D11_BIND_VERTEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            m_vertexBuffer.put()));

    // Create indices: 
    // - each quad has two triangles
    // - each triangle has three vertices
    // - each quad is duplicated for the top and the bottom face of the grid
    m_indexCount = 2 * quadCountHoriz * quadCountDepth * 2 * 3;

    std::vector<uint16_t> indices(m_indexCount);
    size_t k = 0;

    for (auto i = 0; i < quadCountDepth; ++i)
    {
        for (auto j = 0; j < quadCountHoriz; ++j)
        {
            // Compute four indices of a single quad composed of two triangles: ABD and ADC. 
            // The bottom face of the grid has the same indices but in opposite order.
            //
            //     a----b   alternated   a----b
            //     |\   |                |   /|  
            //     | \  |                |  / |
            //     |  \ |                | /  |
            //     |   \|                |/   |
            //     c----d                c----d
            //
            // 1st row: /\/\/\ etc.
            // 2nd row: \/\/\/ etc.
            // 3rd row: /\/\/\ etc.
            // (j+1) % 2 - alternates index order within a row
            // (i+1+j) % 2 - alternates index order in every other row by starting from / or \
            //
            uint16_t a = j + i * n;
            uint16_t b = j + 1 + i * n;
            uint16_t c = j + (i + 1) * n;
            uint16_t d = j + 1 + (i + 1) * n;

            if ((i + 1 + j) % 2)
            {
                // top
                indices[k] = a;
                indices[k + 1] = b;
                indices[k + 2] = d;
                indices[k + 3] = a;
                indices[k + 4] = d;
                indices[k + 5] = c;
                k += 6;

                // bottom
                indices[k] = a;
                indices[k + 1] = d;
                indices[k + 2] = b;
                indices[k + 3] = a;
                indices[k + 4] = c;
                indices[k + 5] = d;
                k += 6;
            }
            else
            {
                // top
                indices[k] = a;
                indices[k + 1] = b;
                indices[k + 2] = c;
                indices[k + 3] = c;
                indices[k + 4] = b;
                indices[k + 5] = d;
                k += 6;

                // botom
                indices[k] = a;
                indices[k + 1] = c;
                indices[k + 2] = b;
                indices[k + 3] = c;
                indices[k + 4] = d;
                indices[k + 5] = b;
                k += 6;
            }
        };
    }

    ASSERT(k == m_indexCount);

    // Create index buffer and load indices to the buffer.
    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(indices.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_indexBuffer.put()));
}

void GridMesh::SetBuffers()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Each vertex is one instance of the VertexPositionColor struct.
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    // Each index is one 16-bit unsigned integer (short).
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
}

void GridMesh::Draw()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->DrawIndexed(m_indexCount, 0, 0);
}

void GridMesh::ReleaseResources()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}
