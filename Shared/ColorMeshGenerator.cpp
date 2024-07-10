#include "pch.h"

#include "ColorMeshGenerator.h"
#include "Utilities.h"

using namespace DirectX;

ColorMeshGenerator::ColorMeshGenerator(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

/// <summary>
/// Creates a unit cube i.e., a cube whose sides are 1 unit long.
/// </summary>
void ColorMeshGenerator::CreateCube(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 8); // the cube has 8 vertices

    m_vertices[i++] = { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) };

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

    info.IndexCount = (uint32_t)indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

    m_meshes[name] = info;
}

/// <summary>
/// Creates a pyramid. The pyramid's base is a unit square.
/// 
/// [Luna] Ex.4 p.242 Construct the vertex and index list of a pyramid.
/// </summary>
void ColorMeshGenerator::CreatePyramid(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 5); // the pyramid has 5 vertices

    const float l = 0.5f;
    m_vertices[i++] = { XMFLOAT3(0.0f, l, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT3(0.0f, 1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT3(0.0f, 1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT3(0.0f, 1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT3(0.0f, 1.0f, 0.0f) };

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

    info.IndexCount = (uint32_t)indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

    m_meshes[name] = info;
}

void ColorMeshGenerator::CopyIndices(std::vector<uint32_t> const& indices, uint32_t startIndexLocation, size_t indexCount)
{
    m_indices.resize(startIndexLocation + indexCount);

    for (size_t i = 0; i < indexCount; ++i)
        m_indices[startIndexLocation + i] = indices[i];
}

/// <summary>
/// Based on [Luna]
/// 
/// Creates a cylinder centered at the origin and parallel to the y-axis. The cylinder is composed of
/// stacks placed vertically one on another. The bottom stack has a cap as its base. Similarily,
/// the top stack has a cap as its top. We build the cylinder starting from the bottom stack.
/// 
/// An example of a cylinder with 3 stacks:
///    ____
///   /    \
///  /      \
/// /________\
///
/// Note that in the above cylinder the radii of the top and bottom caps differ.
/// </summary>
/// <param name="bottomRadius">The radius of the bottom cap</param>
/// <param name="topRadius">The radius of the top cap</param>
/// <param name="cylinderHeight">The cylider's height</param>
/// <param name="sliceCount">The number of slices. A slice is one triangle in the top or the bottom cap.</param>
/// <param name="stackCount">The number of stacks. A stack is one vertical segment of the cylinder.</param>
void ColorMeshGenerator::CreateCylinder(std::string const& name, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    float stackHeight = cylinderHeight / stackCount;

    // Calculate the "top" angle of a single slice triangle.
    float theta = XM_2PI / sliceCount;

    // Calculate the difference between the radii of two consecutive stacks.
    // This difference may be positive, negative, or zero depending on the relative
    // sizes of the top and bottom caps.
    float radiusDelta = (topRadius - bottomRadius) / stackCount;

    // Generate vertices for each stack starting at the bottom stack. We use <= rater than <
    // because we want to generate vertices for the top of the last (top) stack.
    for (uint32_t i = 0; i <= stackCount; ++i)
    {
        // Calculate the y-coordinate of the i-th stack base (or the top of the last stack).
        float y = -0.5f * cylinderHeight + i * stackHeight; 

        // Calculate the radius of the i-th stack base (or the radius of the top of the last stack).
        float r = bottomRadius + i * radiusDelta;

        // Create vertices for the i-th stack. Note that we duplicate the first vertex as 
        // the last one by using <= rather than < 
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float x = r * cosf(j * theta);
            float z = r * sinf(j * theta);

            VertexPositionColor v;
            v.Position = XMFLOAT3(x, y, z);

            // Alternate color for each stack.
            if (i % 2)
                v.Color = XMFLOAT3(1.0f, 0.2f, 0.0f);
            else
                v.Color = XMFLOAT3(0.219f, 0.254f, 0.717f);

            m_vertices.push_back(v);
        }
    }

    // Increase the number of vertices per stack by one because we duplicated the first vertex.
    auto n = sliceCount + 1; 

    // Calculate indices for each stack.
    for (uint32_t i = 0; i < stackCount; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
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

    BuildCylinderTopCap(info.BaseVertexLocation, topRadius, cylinderHeight, sliceCount);
    BuildCylinderBottomCap(info.BaseVertexLocation, bottomRadius, cylinderHeight, sliceCount);

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

void ColorMeshGenerator::BuildCylinderTopCap(uint32_t baseVertexLocation, float topRadius, float cylinderHeight, uint32_t sliceCount)
{
    uint32_t baseIndex = (uint32_t)m_vertices.size() - baseVertexLocation;

    float y = 0.5f * cylinderHeight; // the y-coordinate of the top cap
    float theta = XM_2PI / sliceCount;

    VertexPositionColor v;
    v.Color = XMFLOAT3(0.0f, 0.5f, 1.0f);

    // Duplicate top cap vertices because the texture coordinates and normals differ.
    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float x = topRadius * cosf(i * theta);
        float z = topRadius * sinf(i * theta);

        v.Position = XMFLOAT3(x, y, z);
        m_vertices.push_back(v);

    }

    // The center vertex of the top cap.
    v.Position = XMFLOAT3(0.0f, y, 0.0f);
    v.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
    m_vertices.push_back(v);

    // The index of the center vertex.
    uint32_t centerIndex = (uint32_t)m_vertices.size() - baseVertexLocation - 1;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(centerIndex);
        m_indices.push_back(baseIndex + i + 1);
        m_indices.push_back(baseIndex + i);
    }
}

void ColorMeshGenerator::BuildCylinderBottomCap(uint32_t baseVertexLocation, float bottomRadius, float cylinderHeight, uint32_t sliceCount)
{
    uint32_t baseIndex = (uint32_t)m_vertices.size() - baseVertexLocation;

    float y = -0.5f * cylinderHeight; // the y-coordinate of the bottom cap
    float theta = XM_2PI / sliceCount;

    VertexPositionColor v;
    v.Color = XMFLOAT3(0.0f, 0.5f, 1.0f);

    // Duplicate top cap vertices because the texture coordinates and normals differ.
    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float x = bottomRadius * cosf(i * theta);
        float z = bottomRadius * sinf(i * theta);

        v.Position = XMFLOAT3(x, y, z);
        m_vertices.push_back(v);
    }

    // The center vertex of the bottom cap.
    v.Position = XMFLOAT3(0.0f, y, 0.0f);
    v.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
    m_vertices.push_back(v);

    // The index of the center vertex.
    uint32_t centerIndex = (uint32_t)m_vertices.size() - baseVertexLocation - 1;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(centerIndex);
        m_indices.push_back(baseIndex + i);
        m_indices.push_back(baseIndex + i + 1);
    }
}

/// <summary>
/// Based on [Luna]
/// 
/// Creates a sphere using an approach similar to creating a cylinder.
/// We use trigonometric functions to calculate the radius per stack.
/// Note that the triangles of the sphere do not have equal areas.
/// </summary>
/// <param name="radius">The sphere's radius</param>
/// <param name="sliceCount">The number of slices</param>
/// <param name="stackCount">The number of stacks</param>
void ColorMeshGenerator::CreateSphere(std::string const& name, float radius, uint32_t sliceCount, uint32_t stackCount)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    // Create the sphere's poles.
    VertexPositionColor topVertex{ { 0.0f, radius, 0.0f }, { 1.0f, 1.0f, 1.0f } };
    VertexPositionColor bottomVertex{ { 0.0f, -radius, 0.0f }, { 1.0f, 1.0f, 1.0f } };

    // Add the north pole as the first vertex.
    m_vertices.push_back(topVertex);

    float phiStep = XM_PI / stackCount;
    float thetaStep = XM_2PI / sliceCount;

    // Compute vertices for each stack.
    for (uint32_t i = 0; i <= stackCount; ++i)
    {
        float phi = (i+1) * phiStep;

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            VertexPositionColor v;

            // Convert spherical coordinates to Cartesian coordinates.
            v.Position.x = radius * sinf(phi) * cosf(theta);
            v.Position.y = radius * cosf(phi);
            v.Position.z = radius * sinf(phi) * sinf(theta);

            // Alternate color for each stack.
            if (i % 2)
                v.Color = XMFLOAT3(1.0f, 0.9f, 0.0f);
            else
                v.Color = XMFLOAT3(0.0f, 0.1f, 1.0f);

            m_vertices.push_back(v);
        }
    }

    // Add the south pole as the last vertex.
    m_vertices.push_back(bottomVertex);

    // Compute indices for the top stack which contains the north pole.
    for (uint32_t i = 1; i <= sliceCount; ++i)
    {
        m_indices.push_back(0);
        m_indices.push_back(i + 1);
        m_indices.push_back(i);
    }

    // Compute indices for inner stacks.
    auto baseIndex = 1; // skip the north pole vertex
    auto n = sliceCount + 1; // the number of vertices in a stack
    for (uint32_t i = 0; i < stackCount - 2; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            m_indices.push_back(baseIndex + i * n + j);
            m_indices.push_back(baseIndex + i * n + j + 1);
            m_indices.push_back(baseIndex + (i + 1) * n + j);

            m_indices.push_back(baseIndex + (i + 1) * n + j);
            m_indices.push_back(baseIndex + i * n + j + 1);
            m_indices.push_back(baseIndex + (i + 1) * n + j + 1);
        }
    }

    // Compute indices for the bottom stack which contains the south pole.
    uint32_t southPoleIndex = (uint32_t)m_vertices.size() - info.BaseVertexLocation - 1;

    // Offset the indices to the index of the first vertex in the last stack.
    baseIndex = southPoleIndex - n;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(southPoleIndex);
        m_indices.push_back(baseIndex + i);
        m_indices.push_back(baseIndex + i + 1);
    }

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

/// <summary>
/// Based on [Luna]
/// 
/// Creates a geosphere - a sphere in which each triangle has the same area and equal side lengths.
/// The geosphere generator starts with an icosahedron (a polyhedron with 20 faces) and subdivides
/// its sides (the triangles) into smaller triangles. Then, it projects the new vertices onto
/// a sphere. The process is repeated to improve tessellation.
/// 
/// This is how a single triangle is subdivided into four equal sized triangles:
/// 
///            v1
///             *
///            / \
///           /   \
///       m0 *-----*m1
///         / \   / \
///        /   \ /   \
///       *-----*-----*
///      v0    m2     v2
///
/// </summary>
/// <param name="radius">The sphere's radius</param>
/// <param name="subdivisionCount"></param>
void ColorMeshGenerator::CreateGeosphere(std::string const& name, float radius, uint16_t subdivisionCount)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    // Limit the number of subdivisions to 5.
    subdivisionCount = std::min(subdivisionCount, (uint16_t)5);

    // Define geometry of in icosahedron with 12 vertices and 60 indices.
    // The icosahedron is the starting point to generate the geosphere.
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

    // Tessellate the icosahedron.
    for (auto i = 0; i < subdivisionCount; ++i)
        // The vertices and indices collections are modified in Subdivide.
        Subdivide(vertices, indices); 

    // Project vertices onto a unit sphere and scale.
    for (uint32_t i = 0; i < vertices.size(); ++i)
    {
        XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&vertices[i].Position));
        XMVECTOR p = radius * n;

        XMStoreFloat3(&vertices[i].Position, p);

        if (i % 4)
            vertices[i].Color = XMFLOAT3(0.0f, 0.0f, 1.0f);
        else
            vertices[i].Color = XMFLOAT3(1.0f, 0.0f, 0.0f);
    }

    // Add the geosphere vertices and indices to the buffers.
    for (const auto& v : vertices)
        m_vertices.push_back(v);
    for (auto i : indices)
        m_indices.push_back(i);

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

void ColorMeshGenerator::Subdivide(std::vector<VertexPositionColor>& vertices, std::vector<uint32_t>& indices)
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

    auto triangleCount = indicesCopy.size() / 3;
    for (uint32_t i = 0; i < triangleCount; ++i)
    {
        VertexPositionColor v0 = verticesCopy[indicesCopy[i * 3 + 0]];
        VertexPositionColor v1 = verticesCopy[indicesCopy[i * 3 + 1]];
        VertexPositionColor v2 = verticesCopy[indicesCopy[i * 3 + 2]];

        // Calculate the triangle midpoints.
        VertexPositionColor m0, m1, m2;

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

        // Populate vertex and index collections.
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
void ColorMeshGenerator::CreateGrid(std::string const& name, float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    float dx = gridWidth / quadCountHoriz; // the quad spacing along the x-axis 
    float dz = gridDepth / quadCountDepth; // the quad spacing along the z-axis
    float halfWidth = 0.5f * gridWidth;
    float halfDepth = 0.5f * gridDepth;

    // The grid is built from an M x N matrix of vertices. 
    uint32_t m = quadCountDepth + 1;
    uint32_t n = quadCountHoriz + 1;

    // Create vertices.
    uint32_t vertexCount = m * n;
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
    float z = halfDepth;
    for (uint32_t i = 0; i < m; ++i)
    {
        auto x = -halfWidth;

        for (uint32_t j = 0; j < n; ++j)
        {
            auto k = i * n + j;

            m_vertices[info.BaseVertexLocation + k].Position = XMFLOAT3(x, 0, z);
            m_vertices[info.BaseVertexLocation + k].Color = XMFLOAT3(0.6f, 0.6f, 0.6f);

            x += dx;
        };

        z -= dz;
    }

    // Create indices: 
    // - each quad has two triangles
    // - each triangle has three vertices
    // - each quad is duplicated for the top and the bottom face of the grid
    uint32_t indexCount = 2 * quadCountHoriz * quadCountDepth * 2 * 3;
    m_indices.resize(info.StartIndexLocation + indexCount);
    size_t k = info.StartIndexLocation;

    for (uint32_t i = 0; i < quadCountDepth; ++i)
    {
        for (uint32_t j = 0; j < quadCountHoriz; ++j)
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
            uint32_t a = j + i * n;
            uint32_t b = j + 1 + i * n;
            uint32_t c = j + (i + 1) * n;
            uint32_t d = j + 1 + (i + 1) * n;

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

    m_meshes[name] = info;
}

void ColorMeshGenerator::CreateBuffers()
{
    // Create an immutable vertex buffer and load data.
    m_vertexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_VERTEX_BUFFER,
            (uint32_t)m_vertices.size() * sizeof(VertexPositionColor),
            m_vertices.data()));

    // Create an immutable index buffer and load indices to the buffer.
    m_indexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_INDEX_BUFFER,
            (uint32_t)m_indices.size() * sizeof(uint32_t),
            m_indices.data()));
}

void ColorMeshGenerator::SetBuffers()
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

void ColorMeshGenerator::DrawMesh(std::string const& name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    const auto& info = m_meshes[name];

    // Draw one object at a time as each object may have a different world matrix.
    context->DrawIndexed(info.IndexCount, info.StartIndexLocation, info.BaseVertexLocation);
}

void ColorMeshGenerator::Clear()
{
    // Clear collections.
    m_vertices.clear();
    m_indices.clear();
    m_meshes.clear();

    // Release buffers.
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

