#include "pch.h"

#include "MeshFactory.h"
#include "Utilities.h"

using namespace DirectX;

MeshFactory::MeshFactory(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

void MeshFactory::MakeCube()
{
    const uint16_t CubeVertexCount = 8;

    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size();
    info.StartIndexLocation = m_indices.size();

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

    std::vector<uint32_t> indices =
    {
        0, 1, 2, // -x
        1, 3, 2,

        4, 6, 5, // +x
        5, 6, 7,

        0, 5, 1, // -y
        0, 4, 5,

        2, 7, 6, // +y
        2, 3, 7,

        0, 6, 4, // -z
        0, 2, 6,

        1, 7, 3, // +z
        1, 5, 7,
    };

    info.IndexCount = indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

    m_meshes.push_back(info);
}

void MeshFactory::MakePyramid()
{
    const uint16_t PyramidVertexCount = 5;

    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size();
    info.StartIndexLocation = m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + PyramidVertexCount);

    const float l = 0.5f;
    m_vertices[i++] = { XMFLOAT3(0.0f, l, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };

    ASSERT(m_vertices.size() == i);

    std::vector<uint32_t> indices =
    {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 1,

        2, 1, 3,
        3, 1, 4,
    };

    info.IndexCount = indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

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

/// <summary>
/// Based on [Luna]
/// Creates a sphere mesh using an approach similar to creating a cylinder mesh.
/// We use trigonometric functions to calculate the radius per ring.
/// The triangles of the sphere do not have equal areas.
/// </summary>
/// <param name="radius">The sphere's radius</param>
/// <param name="sliceCount">The number of slices</param>
/// <param name="stackCount">The number of stacks</param>
void MeshFactory::MakeSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size();
    info.StartIndexLocation = m_indices.size();

    // Compute the vertices stating at the top pole and moving down the stacks.

    // Create poles.
    VertexPositionColor topVertex{ { 0.0f, radius, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
    VertexPositionColor bottomVertex{ { 0.0f, -radius, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    m_vertices.push_back(topVertex);

    float phiStep = XM_PI / stackCount;
    float thetaStep = XM_2PI / sliceCount;

    // Compute vertices for each stack ring (do not count the poles as rings).
    for (auto i = 0; i <= stackCount; ++i)
    {
        float phi = (i+1) * phiStep;

        // Vertices of a ring.
        for (auto j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            VertexPositionColor v;

            // spherical to cartesian
            v.Position.x = radius * sinf(phi) * cosf(theta);
            v.Position.y = radius * cosf(phi);
            v.Position.z = radius * sinf(phi) * sinf(theta);

            // Alternate color for each stack.
            if (i % 2)
                v.Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
            else
                v.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

            m_vertices.push_back(v);
        }
    }

    m_vertices.push_back(bottomVertex);

    // Compute indices for top stack.  
    // The top stack was written first to the vertex buffer and connects the top pole to the first ring.
    for (auto i = 1; i <= sliceCount; ++i)
    {
        m_indices.push_back(0);
        m_indices.push_back(i + 1);
        m_indices.push_back(i);
    }

    // Compute indices for inner stacks (not connected to poles).
    // Offset the indices to the index of the first vertex in the first ring.
    // This skips the top pole vertex.
    auto baseIndex = 1;
    auto n = sliceCount + 1; // the number of vertices in a ring
    for (auto i = 0; i < stackCount - 2; ++i)
    {
        for (auto j = 0; j < sliceCount; ++j)
        {
            m_indices.push_back(baseIndex + i * n + j);
            m_indices.push_back(baseIndex + i * n + j + 1);
            m_indices.push_back(baseIndex + (i + 1) * n + j);

            m_indices.push_back(baseIndex + (i + 1) * n + j);
            m_indices.push_back(baseIndex + i * n + j + 1);
            m_indices.push_back(baseIndex + (i + 1) * n + j + 1);
        }
    }

    // Compute indices for the bottom stack. 
    // The bottom stack was written last to the vertex buffer and connects the bottom pole to the bottom ring.

    // The south pole vertex was added last.
    uint32_t southPoleIndex = (uint32_t)m_vertices.size() - info.BaseVertexLocation - 1;

    // Offset the indices to the index of the first vertex in the last ring.
    baseIndex = southPoleIndex - n;

    for (auto i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(southPoleIndex);
        m_indices.push_back(baseIndex + i);
        m_indices.push_back(baseIndex + i + 1);
    }

    info.IndexCount = m_indices.size() - info.StartIndexLocation;

    m_meshes.push_back(info);
}

/*
    Based on [Luna]

    A geosphere approximates a sphere using triangles with almost equal areas as well as equal side lengths.
    To generate a geosphere, we start with an icosahedron (a polyhedron with 20 faces), subdivide the triangles,
    and then project the new vertices onto the sphere with the given radius. We repeat this process to improve
    the tessellation.

    A triangle can be subdivided into four equal sized triangles:

            v1
             *
            / \
           /   \
        m0*-----*m1
         / \   / \
        /   \ /   \
       *-----*-----*
      v0    m2     v2

    The new vertices are found by taking the midpoints along the edges of the original triangle.
    The new vertices can then be projected onto a sphere of radius r by projecting the vertices
    onto the unit sphere and then scalar multiplying by r:

               v
    v' = r * -----
             ||v||
*/
void MeshFactory::MakeGeosphere(float radius, uint16_t numSubdivisions)
{
    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size(); // initial vertex count
    info.StartIndexLocation = m_indices.size(); // initial index count

    // Put a cap on the number of subdivisions.
    numSubdivisions = std::min(numSubdivisions, (uint16_t)5);

    // Define vertices and indices of icosahedron with 12 vertices and 60 indices.
    std::vector<VertexPositionColor> vertices(12);
    std::vector<uint32_t> indices(60);

    const float X = 0.525731f;
    const float Z = 0.850651f;

    XMFLOAT3 pos[12] =
    {
        XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),
        XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
        XMFLOAT3(0.0f, Z, X), XMFLOAT3(0.0f, Z, -X),
        XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X),
        XMFLOAT3(Z, X, 0.0f), XMFLOAT3(-Z, X, 0.0f),
        XMFLOAT3(Z, -X, 0.0f), XMFLOAT3(-Z, -X, 0.0f)
    };

    uint32_t k[60] =
    {
        1, 4, 0, 4, 9, 0, 4, 5, 9, 8, 5, 4, 1, 8, 4,
        1, 10, 8, 10, 3, 8, 8, 3, 5, 3, 2, 5, 3, 7, 2,
        3, 10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6, 1, 0,
        10, 1, 6, 11, 0, 9, 2, 11, 9, 5, 2, 9, 11, 2, 7
    };

    for (auto i = 0; i < 12; ++i)
        vertices[i].Position = pos[i];

    for (auto i = 0; i < 60; ++i)
        indices[i] = k[i];

    // Approximate a sphere by tessellating an icosahedron.
    for (auto i = 0; i < numSubdivisions; ++i)
        Subdivide(vertices, indices); // vertices and indices collections are modified in Subdivide

    // Project vertices onto sphere and scale.
    for (auto i = 0; i < vertices.size(); ++i)
    {
        // Project onto unit sphere.
        XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&vertices[i].Position));

        // Project onto sphere.
        XMVECTOR p = radius * n;

        XMStoreFloat3(&vertices[i].Position, p);

        if (i % 4)
            vertices[i].Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
        else
            vertices[i].Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // Add the geosphere vertices and indices to the buffers.
    for (const auto& v : vertices)
        m_vertices.push_back(v);
    for (auto i : indices)
        m_indices.push_back(i);

    info.IndexCount = m_indices.size() - info.StartIndexLocation;

    m_meshes.push_back(info);
}

void MeshFactory::Subdivide(std::vector<VertexPositionColor>& vertices, std::vector<uint32_t>& indices)
{
    // Save a copy of the input geometry.
    std::vector<VertexPositionColor> verticesCopy(vertices);
    std::vector<uint32_t> indicesCopy(indices);

    // Clear the original vectors. We will re-populate them with new vertices and indices.
    vertices.clear();
    indices.clear();

    //       v1
    //        *
    //       / \
    //      /   \
    //   m0*-----*m1
    //    / \   / \
    //   /   \ /   \
    //  *-----*-----*
    // v0    m2     v2

    auto numTris = indicesCopy.size() / 3;
    for (auto i = 0; i < numTris; ++i)
    {
        VertexPositionColor v0 = verticesCopy[indicesCopy[i * 3 + 0]];
        VertexPositionColor v1 = verticesCopy[indicesCopy[i * 3 + 1]];
        VertexPositionColor v2 = verticesCopy[indicesCopy[i * 3 + 2]];

        //
        // Generate the midpoints.
        //

        VertexPositionColor m0, m1, m2;

        // For subdivision, we just care about the position component.  We derive the other
        // vertex components in CreateGeosphere.

        m0.Position = XMFLOAT3(
            0.5f * (v0.Position.x + v1.Position.x),
            0.5f * (v0.Position.y + v1.Position.y),
            0.5f * (v0.Position.z + v1.Position.z));

        m1.Position = XMFLOAT3(
            0.5f * (v1.Position.x + v2.Position.x),
            0.5f * (v1.Position.y + v2.Position.y),
            0.5f * (v1.Position.z + v2.Position.z));

        m2.Position = XMFLOAT3(
            0.5f * (v0.Position.x + v2.Position.x),
            0.5f * (v0.Position.y + v2.Position.y),
            0.5f * (v0.Position.z + v2.Position.z));

        //
        // Add new geometry.
        //

        vertices.push_back(v0); // 0
        vertices.push_back(v1); // 1
        vertices.push_back(v2); // 2
        vertices.push_back(m0); // 3
        vertices.push_back(m1); // 4
        vertices.push_back(m2); // 5

        indices.push_back(i * 6 + 0);
        indices.push_back(i * 6 + 3);
        indices.push_back(i * 6 + 5);

        indices.push_back(i * 6 + 3);
        indices.push_back(i * 6 + 4);
        indices.push_back(i * 6 + 5);

        indices.push_back(i * 6 + 5);
        indices.push_back(i * 6 + 4);
        indices.push_back(i * 6 + 2);

        indices.push_back(i * 6 + 3);
        indices.push_back(i * 6 + 1);
        indices.push_back(i * 6 + 4);
    }
}

/// <summary>
/// Builds a grid mesh in the xz-plane procedurally.
/// </summary>
/// <param name="gridWidth">Grid width. It determines the relative size of the grid.</param>
/// <param name="gridDepth">Grid depth. It determines the relative size of the grid.</param>
/// <param name="quadCountHoriz">The number of quads in the grid in the horizontal dimension (x-axis)</param>
/// <param name="quadCountDepth">The number of quads in the grid in the depth dimension (z-axis)</param>
void MeshFactory::MakeGrid(float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth)
{
    MeshInfo info;
    info.BaseVertexLocation = m_vertices.size(); // initial vertex count
    info.StartIndexLocation = m_indices.size(); // initial index count

    auto colorBlue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
    auto dx = gridWidth / quadCountHoriz; // the quad spacing along the x-axis 
    auto dz = gridDepth / quadCountDepth; // the quad spacing along the z-axis
    auto halfWidth = 0.5 * gridWidth;
    auto halfDepth = 0.5 * gridDepth;

    // The grid is built from an M x N matrix of vertices. 
    auto m = quadCountDepth + 1;
    auto n = quadCountHoriz + 1;

    // Create vertices.
    auto vertexCount = m * n;
    m_vertices.resize(info.BaseVertexLocation + vertexCount);

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

            m_vertices[info.BaseVertexLocation + k].Position = XMFLOAT3(x, 0, z);
            m_vertices[info.BaseVertexLocation + k].Color = colorBlue;

            x += dx;
        };

        z -= dz;
    }

    // Create indices: 
    // - each quad has two triangles
    // - each triangle has three vertices
    // - each quad is duplicated for the top and the bottom face of the grid
    auto indexCount = 2 * quadCountHoriz * quadCountDepth * 2 * 3;
    m_indices.resize(info.StartIndexLocation + indexCount);
    size_t k = info.StartIndexLocation;

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
            m_indices[k] = a;
            m_indices[k + 1] = b;
            m_indices[k + 2] = d;
            m_indices[k + 3] = a;
            m_indices[k + 4] = d;
            m_indices[k + 5] = c;
            k += 6;

            // bottom face
            m_indices[k] = a;
            m_indices[k + 1] = d;
            m_indices[k + 2] = b;
            m_indices[k + 3] = a;
            m_indices[k + 4] = c;
            m_indices[k + 5] = d;
            k += 6;
        };
    }

    ASSERT(k - info.StartIndexLocation == indexCount);

    info.IndexCount = indexCount;

    m_meshes.push_back(info);
}

void MeshFactory::Build()
{
    // Create an immutable vertex buffer and load data.
    m_vertexBuffer.attach(
        CreateImmutableVertexBuffer(
            m_deviceResources->GetD3DDevice(),
            m_vertices.size() * sizeof(VertexPositionColor),
            m_vertices.data()));

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
    const auto& info = m_meshes[index];

    context->DrawIndexed(info.IndexCount, info.StartIndexLocation, info.BaseVertexLocation);
}

void MeshFactory::Release()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

void MeshFactory::CopyIndices(std::vector<uint32_t> const& indices, uint32_t startIndexLocation, size_t indexCount)
{
    m_indices.resize(startIndexLocation + indexCount);

    for (auto i = 0; i < indexCount; ++i)
    {
        m_indices[startIndexLocation + i] = indices[i];
    }
}
