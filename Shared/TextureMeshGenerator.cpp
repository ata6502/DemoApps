#include "pch.h"
#include <cmath>

#include "TextureMeshGenerator.h"
#include "Utilities.h"

using namespace DirectX;

TextureMeshGenerator::TextureMeshGenerator(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

/// <summary>
/// Creates a unit cube with 24 vertices. This is sufficient to define 
/// textures on each cube's face.
/// </summary>
void TextureMeshGenerator::CreateCube(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 24);

    float l = 0.5f, n = 1.0f;

    // front face
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l,  l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(1.0f, 1.0f) };

    // back face
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT3(0, 0, n), XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT3(0, 0, n), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  l,  l), XMFLOAT3(0, 0, n), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-l,  l,  l), XMFLOAT3(0, 0, n), XMFLOAT2(1.0f, 0.0f) };

    // top face
    m_vertices[i++] = { XMFLOAT3(-l,  l, -l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l,  l,  l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  l,  l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  l, -l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 1.0f) };

    // bottom face
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT3(0, -n, 0), XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT3(0, -n, 0), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT3(0, -n, 0), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT3(0, -n, 0), XMFLOAT2(1.0f, 0.0f) };

    // left face
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT3(-n, 0, 0), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l,  l,  l), XMFLOAT3(-n, 0, 0), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-l,  l, -l), XMFLOAT3(-n, 0, 0), XMFLOAT2(1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT3(-n, 0, 0), XMFLOAT2(1.0f, 1.0f) };

    // right face
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT3(n, 0, 0), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  l, -l), XMFLOAT3(n, 0, 0), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  l,  l), XMFLOAT3(n, 0, 0), XMFLOAT2(1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT3(n, 0, 0), XMFLOAT2(1.0f, 1.0f) };

    ASSERT(m_vertices.size() == i);

    std::vector<uint32_t> indices =
    {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 5, 6,
        4, 6, 7,

        // top face
        8, 9, 10,
        8, 10, 11,

        // bottom face
        12, 13, 14,
        12, 14, 15,

        // left face
        16, 17, 18,
        16, 18, 19,

        // right face
        20, 21, 22,
        20, 22, 23,
    };

    info.IndexCount = (uint32_t)indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

    m_meshes[name] = info;
}

/// <summary>
/// Creates a unit cube i.e., a cube whose sides are 1 unit long. The simple cube has 8 vertices
/// which is not sufficient to define a texture on every face.
/// </summary>
void TextureMeshGenerator::CreateSimpleCube(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 8); // a cube has 8 vertices

    float l = 0.5f, n = 1.0f / sqrtf(3.0f); // all coordinates of the normals have the value n
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT3(-n, -n, -n) };
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT3(-n, -n,  n) };
    m_vertices[i++] = { XMFLOAT3(-l,  l, -l), XMFLOAT3(-n,  n, -n) };
    m_vertices[i++] = { XMFLOAT3(-l,  l,  l), XMFLOAT3(-n,  n,  n) };
    m_vertices[i++] = { XMFLOAT3(l, -l, -l), XMFLOAT3(n, -n, -n) };
    m_vertices[i++] = { XMFLOAT3(l, -l,  l), XMFLOAT3(n, -n,  n) };
    m_vertices[i++] = { XMFLOAT3(l,  l, -l), XMFLOAT3(n,  n, -n) };
    m_vertices[i++] = { XMFLOAT3(l,  l,  l), XMFLOAT3(n,  n,  n) };

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
/// Creates a pyramid with 18 vertices. This is sufficient to define 
/// textures on each pyramid's face.
/// </summary>
void TextureMeshGenerator::CreatePyramid(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 18); // a pyramid with additonal vertices has 18 vertices

    // vertices
    float l = 0.5f;
    XMFLOAT3 v0 = {  0,  l,  0 };
    XMFLOAT3 v1 = { -l, -l, -l };
    XMFLOAT3 v2 = {  l, -l, -l };
    XMFLOAT3 v3 = {  l, -l,  l };
    XMFLOAT3 v4 = { -l, -l,  l };

    // normals
    XMFLOAT3 n0; XMStoreFloat3(&n0, Utilities::ComputeNormal(v0, v2, v1)); // front
    XMFLOAT3 n1; XMStoreFloat3(&n1, Utilities::ComputeNormal(v0, v3, v2)); // right
    XMFLOAT3 n2; XMStoreFloat3(&n2, Utilities::ComputeNormal(v0, v4, v3)); // back
    XMFLOAT3 n3; XMStoreFloat3(&n3, Utilities::ComputeNormal(v0, v1, v4)); // left
    XMFLOAT3 n4; XMStoreFloat3(&n4, Utilities::ComputeNormal(v1, v2, v3)); // bottom
    XMFLOAT3 n5; XMStoreFloat3(&n5, Utilities::ComputeNormal(v1, v3, v4)); // bottom (should be the same as n4)

    // front
    m_vertices[i++] = { v0, n0, XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { v2, n0, XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { v1, n0, XMFLOAT2(0.5f, 0.0f) };

    // right
    m_vertices[i++] = { v0, n1, XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { v3, n1, XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { v2, n1, XMFLOAT2(0.5f, 0.0f) };

    // back
    m_vertices[i++] = { v0, n2, XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { v4, n2, XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { v3, n2, XMFLOAT2(0.5f, 0.0f) };

    // left
    m_vertices[i++] = { v0, n3, XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { v1, n3, XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { v4, n3, XMFLOAT2(0.5f, 0.0f) };

    // bottom - one half (bottom is a rectangle composed of two triangles)
    m_vertices[i++] = { v1, n4, XMFLOAT2(1.0f, 1.0f) };
    m_vertices[i++] = { v2, n4, XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { v3, n4, XMFLOAT2(0.5f, 0.0f) };

    // bottom- another half
    m_vertices[i++] = { v1, n5 };
    m_vertices[i++] = { v3, n5 };
    m_vertices[i++] = { v4, n5 };

    ASSERT(m_vertices.size() == i);

    std::vector<uint32_t> indices =
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,

        12, 13, 14,
        15, 16, 17
    };

    info.IndexCount = (uint32_t)indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

    m_meshes[name] = info;
}

/// <summary>
/// Creates a pyramid. The pyramid's base is a unit square. The simple pyramid has 5 vertices
/// which is not sufficient to define a texture on every face.
/// [Luna] Ex.4 p.242 Construct the vertex and index list of a pyramid.
/// </summary>
void TextureMeshGenerator::CreateSimplePyramid(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 5); // the pyramid has 5 vertices

    // Define pyramid's vertices.
    const float l = 0.5f;
    float a = sqrt(2.0f);
    m_vertices[i++] = { XMFLOAT3(0.0f, l, 0.0f), XMFLOAT3(0.0f, 0.1f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l, -l,  l), XMFLOAT3( a, -l,  a) };
    m_vertices[i++] = { XMFLOAT3( l, -l, -l), XMFLOAT3( a, -l, -a) };
    m_vertices[i++] = { XMFLOAT3(-l, -l, -l), XMFLOAT3(-a, -l, -a) };
    m_vertices[i++] = { XMFLOAT3(-l, -l,  l), XMFLOAT3(-a, -l,  a) };

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

void TextureMeshGenerator::CopyIndices(std::vector<uint32_t> const& indices, uint32_t startIndexLocation, size_t indexCount)
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
void TextureMeshGenerator::CreateCylinder(std::string const& name, float bottomRadius, float topRadius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
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
        // This is necessary for correct texture rendering.
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float c = cosf(j * theta);
            float s = sinf(j * theta);

            VertexPositionNormalTexture vertex;
            vertex.Position = XMFLOAT3(r * c, y, r * s);

            // Compute texture coordinates for the cylider mesh.
            vertex.Texture.x = (float)j / sliceCount;
            vertex.Texture.y = 1.0f - (float)i / stackCount;

            // Computing Tangent Space Basis Vectors for an Arbitrary Mesh 
            // "Foundations of Game Engine Development, Volume 2: Rendering"

            // Cylinder can be parameterized as follows, where v [0,1] parameter 
            // goes in the same direction as the v tex-coord so that 
            // the bitangent goes in the same direction as the v tex-coord.
            //
            // Let r0 be the bottom radius and let r1 be the top radius.
            //
            //  y(v) = h - hv             (from top to bottom: y(v) [h,0])
            //  r(v) = r1 + (r0-r1)v      (from top radius to bottom radius: r(v) [r1,r0])
            //
            //  x(t, v) = r(v)*cos(t)
            //  y(t, v) = h - hv
            //  z(t, v) = r(v)*sin(t)
            // 
            //  tangent
            //  -------
            //  dx/dt = -r(v)*sin(t)
            //  dy/dt = 0
            //  dz/dt = +r(v)*cos(t)
            //
            //  bitangent
            //  ---------
            //  dx/dv = (r0-r1)*cos(t)
            //  dy/dv = -h
            //  dz/dv = (r0-r1)*sin(t)
 
            // Calculate a unit length tangent.
            XMFLOAT3 tangent = XMFLOAT3(-s, 0.0f, c);

            // Calculate a bitangent.
            float dr = bottomRadius - topRadius;
            XMFLOAT3 bitangent(dr * c, -cylinderHeight, dr * s);

            // Vectors t, b, n are mutually perpendicular. Use cross product to find normal: n = t x b
            XMVECTOR t = XMLoadFloat3(&tangent);
            XMVECTOR b = XMLoadFloat3(&bitangent);
            XMVECTOR n = XMVector3Normalize(XMVector3Cross(t, b));
            XMStoreFloat3(&vertex.Normal, n);

            m_vertices.push_back(vertex);
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

void TextureMeshGenerator::BuildCylinderTopCap(uint32_t baseVertexLocation, float topRadius, float cylinderHeight, uint32_t sliceCount)
{
    uint32_t baseIndex = (uint32_t)m_vertices.size() - baseVertexLocation;

    float y = 0.5f * cylinderHeight; // the y-coordinate of the top cap
    float theta = XM_2PI / sliceCount;

    VertexPositionNormalTexture vertex;

    // Duplicate top cap vertices because the texture coordinates and normals differ.
    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float x = topRadius * cosf(i * theta);
        float z = topRadius * sinf(i * theta);

        // Scale down by the height to make the top cap texture proportional to the base.
        float u = x / cylinderHeight + 0.5f;
        float v = z / cylinderHeight + 0.5f;

        vertex.Position = XMFLOAT3(x, y, z);
        vertex.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertex.Texture = XMFLOAT2(u, v);
        m_vertices.push_back(vertex);

    }

    // The center vertex of the top cap.
    vertex.Position = XMFLOAT3(0.0f, y, 0.0f);
    vertex.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex.Texture = XMFLOAT2(0.5f, 0.5f);
    m_vertices.push_back(vertex);

    // The index of the center vertex.
    uint32_t centerIndex = (uint32_t)m_vertices.size() - baseVertexLocation - 1;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        m_indices.push_back(centerIndex);
        m_indices.push_back(baseIndex + i + 1);
        m_indices.push_back(baseIndex + i);
    }
}

void TextureMeshGenerator::BuildCylinderBottomCap(uint32_t baseVertexLocation, float bottomRadius, float cylinderHeight, uint32_t sliceCount)
{
    uint32_t baseIndex = (uint32_t)m_vertices.size() - baseVertexLocation;

    float y = -0.5f * cylinderHeight; // the y-coordinate of the bottom cap
    float theta = XM_2PI / sliceCount;

    VertexPositionNormalTexture vertex;

    // Duplicate top cap vertices because the texture coordinates and normals differ.
    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float x = bottomRadius * cosf(i * theta);
        float z = bottomRadius * sinf(i * theta);

        // Scale down by the height to make the top cap texture proportional to the base.
        float u = x / cylinderHeight + 0.5f;
        float v = z / cylinderHeight + 0.5f;

        vertex.Position = XMFLOAT3(x, y, z);
        vertex.Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
        vertex.Texture = XMFLOAT2(u, v);
        m_vertices.push_back(vertex);
    }

    // The center vertex of the bottom cap.
    vertex.Position = XMFLOAT3(0.0f, y, 0.0f);
    vertex.Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
    vertex.Texture = XMFLOAT2(0.5f, 0.5f);
    m_vertices.push_back(vertex);

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
void TextureMeshGenerator::CreateSphere(std::string const& name, float radius, uint32_t sliceCount, uint32_t stackCount)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size();
    info.StartIndexLocation = (uint32_t)m_indices.size();

    // Create the sphere's poles.
    VertexPositionNormalTexture topVertex{ { 0.0f, radius, 0.0f }, { 0.0f, 1.0f, 0.0f } };
    VertexPositionNormalTexture bottomVertex{ { 0.0f, -radius, 0.0f }, {0.0f, -1.0f, 0.0f } };

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

            VertexPositionNormalTexture v;

            // Convert spherical coordinates to Cartesian coordinates.
            v.Position.x = radius * sinf(phi) * cosf(theta);
            v.Position.y = radius * cosf(phi);
            v.Position.z = radius * sinf(phi) * sinf(theta);

            // Compute normal vector.
            XMVECTOR p = XMLoadFloat3(&v.Position);
            XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

            // Compute texture coordinates.
            v.Texture.x = theta / XM_2PI;
            v.Texture.y = phi / XM_PI;

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
/// The code to create a geosphere comes from DirectXTK
/// https://github.com/microsoft/DirectXTK/blob/main/Src/Geometry.cpp
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
/// <param name="subdivisionCount">The number of subdivisions between 0 and 5</param>
void TextureMeshGenerator::CreateGeosphere(std::string const& name, float radius, uint16_t subdivisionCount)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());
    ASSERT(subdivisionCount >= 0);

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    // Limit the number of subdivisions to 5.
    subdivisionCount = std::min(subdivisionCount, (uint16_t)5);

    std::vector<VertexPositionNormalTexture> vertices;
    std::vector<uint32_t> indices;

    // An undirected edge between two vertices.
    using UndirectedEdge = std::pair<uint32_t, uint32_t>;

    auto makeUndirectedEdge = [](uint32_t a, uint32_t b) noexcept
    {
        return std::make_pair(std::max(a, b), std::min(a, b));
    };

    using EdgeSubdivisionMap = std::map<UndirectedEdge, uint32_t>;

    static const XMFLOAT3 OctahedronVertices[] =
    {
        XMFLOAT3(0,  1,  0),
        XMFLOAT3(0,  0, -1),
        XMFLOAT3(1,  0,  0),
        XMFLOAT3(0,  0,  1),
        XMFLOAT3(-1,  0,  0),
        XMFLOAT3(0, -1,  0),
    };
    static const uint16_t OctahedronIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4,
        0, 4, 1,
        5, 1, 4,
        5, 4, 3,
        5, 3, 2,
        5, 2, 1
    };

    // Start with an octahedron; copy the data into the vertex/index collection.
    std::vector<XMFLOAT3> vertexPositions(std::begin(OctahedronVertices), std::end(OctahedronVertices));

    indices.insert(indices.begin(), std::begin(OctahedronIndices), std::end(OctahedronIndices));

    constexpr uint32_t northPoleIndex = 0;
    constexpr uint32_t southPoleIndex = 5;

    for (size_t subdivision = 0; subdivision < subdivisionCount; ++subdivision)
    {
        ASSERT(indices.size() % 3 == 0);

        EdgeSubdivisionMap subdividedEdges;

        std::vector<uint32_t> newIndices;

        const size_t triangleCount = indices.size() / 3;
        for (size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle)
        {
            const uint32_t iv0 = indices[iTriangle * 3 + 0];
            const uint32_t iv1 = indices[iTriangle * 3 + 1];
            const uint32_t iv2 = indices[iTriangle * 3 + 2];

            XMFLOAT3 v01;
            XMFLOAT3 v12;
            XMFLOAT3 v20;
            uint32_t iv01;
            uint32_t iv12;
            uint32_t iv20;

            auto const divideEdge = [&](uint32_t i0, uint32_t i1, XMFLOAT3& outVertex, uint32_t& outIndex)
            {
                const UndirectedEdge edge = makeUndirectedEdge(i0, i1);

                auto it = subdividedEdges.find(edge);
                if (it != subdividedEdges.end())
                {
                    outIndex = it->second;
                    outVertex = vertexPositions[outIndex];
                }
                else
                {
                    XMStoreFloat3(
                        &outVertex,
                        XMVectorScale(
                            XMVectorAdd(XMLoadFloat3(&vertexPositions[i0]), XMLoadFloat3(&vertexPositions[i1])),
                            0.5f));

                    outIndex = static_cast<uint32_t>(vertexPositions.size());
                    vertexPositions.push_back(outVertex);

                    auto entry = std::make_pair(edge, outIndex);
                    subdividedEdges.insert(entry);
                }
            };

            divideEdge(iv0, iv1, v01, iv01);
            divideEdge(iv1, iv2, v12, iv12);
            divideEdge(iv0, iv2, v20, iv20);

            // Add the new indices.
            //        v0
            //        o
            //       /a\
            //  v20 o---o v01
            //     /b\c/d\
            // v2 o---o---o v1
            //       v12
            const uint32_t indicesToAdd[] =
            {
                 iv0, iv01, iv20,
                iv20, iv12,  iv2,
                iv20, iv01, iv12,
                iv01,  iv1, iv12,
            };
            newIndices.insert(newIndices.end(), std::begin(indicesToAdd), std::end(indicesToAdd));
        }

        indices = std::move(newIndices);
    }

    // Now that we've completed subdivision, fill in the final vertex collection
    vertices.reserve(vertexPositions.size());
    for (const auto& it : vertexPositions)
    {
        auto const normal = XMVector3Normalize(XMLoadFloat3(&it));
        auto const pos = XMVectorScale(normal, radius);

        XMFLOAT3 normalFloat3;
        XMStoreFloat3(&normalFloat3, normal);

        const float longitude = atan2f(normalFloat3.x, -normalFloat3.z);
        const float latitude = acosf(normalFloat3.y);

        const float u = longitude / XM_2PI + 0.5f;
        const float v = latitude / XM_PI;

        auto const texcoord = XMVectorSet(1.0f - u, v, 0.0f, 0.0f);
        VertexPositionNormalTexture vertex;
        XMStoreFloat3(&vertex.Position, pos);
        XMStoreFloat3(&vertex.Normal, normal);
        XMStoreFloat2(&vertex.Texture, texcoord);
        vertices.push_back(vertex);
    }

    // A texture coordinate wraparound fixup.
    const size_t preFixupVertexCount = vertices.size();
    for (size_t i = 0; i < preFixupVertexCount; ++i)
    {
        const bool isOnPrimeMeridian = XMVector2NearEqual(
            XMVectorSet(vertices[i].Position.x, vertices[i].Texture.x, 0.0f, 0.0f),
            XMVectorZero(),
            XMVectorSplatEpsilon());

        if (isOnPrimeMeridian)
        {
            size_t newIndex = vertices.size();

            VertexPositionNormalTexture v = vertices[i];
            v.Texture.x = 1.0f;
            vertices.push_back(v);

            for (size_t j = 0; j < indices.size(); j += 3)
            {
                uint32_t* triIndex0 = &indices[j + 0];
                uint32_t* triIndex1 = &indices[j + 1];
                uint32_t* triIndex2 = &indices[j + 2];

                if (*triIndex0 == i)
                {
                }
                else if (*triIndex1 == i)
                {
                    std::swap(triIndex0, triIndex1);
                }
                else if (*triIndex2 == i)
                {
                    std::swap(triIndex0, triIndex2);
                }
                else
                {
                    continue;
                }

                ASSERT(*triIndex0 == i);
                ASSERT(*triIndex1 != i && *triIndex2 != i);

                const VertexPositionNormalTexture& v0 = vertices[*triIndex0];
                const VertexPositionNormalTexture& v1 = vertices[*triIndex1];
                const VertexPositionNormalTexture& v2 = vertices[*triIndex2];

                if (abs(v0.Texture.x - v1.Texture.x) > 0.5f ||
                    abs(v0.Texture.x - v2.Texture.x) > 0.5f)
                {
                    *triIndex0 = static_cast<uint32_t>(newIndex);
                }
            }
        }
    }

    // Fix the poles.
    auto const fixPole = [&](size_t poleIndex)
    {
        const auto& poleVertex = vertices[poleIndex];
        bool overwrittenPoleVertex = false;

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t* pPoleIndex;
            uint32_t* pOtherIndex0;
            uint32_t* pOtherIndex1;
            if (indices[i + 0] == poleIndex)
            {
                pPoleIndex = &indices[i + 0];
                pOtherIndex0 = &indices[i + 1];
                pOtherIndex1 = &indices[i + 2];
            }
            else if (indices[i + 1] == poleIndex)
            {
                pPoleIndex = &indices[i + 1];
                pOtherIndex0 = &indices[i + 2];
                pOtherIndex1 = &indices[i + 0];
            }
            else if (indices[i + 2] == poleIndex)
            {
                pPoleIndex = &indices[i + 2];
                pOtherIndex0 = &indices[i + 0];
                pOtherIndex1 = &indices[i + 1];
            }
            else
            {
                continue;
            }

            const auto& otherVertex0 = vertices[*pOtherIndex0];
            const auto& otherVertex1 = vertices[*pOtherIndex1];

            VertexPositionNormalTexture newPoleVertex = poleVertex;
            newPoleVertex.Texture.x = (otherVertex0.Texture.x + otherVertex1.Texture.x) / 2;
            newPoleVertex.Texture.y = poleVertex.Texture.y;

            if (!overwrittenPoleVertex)
            {
                vertices[poleIndex] = newPoleVertex;
                overwrittenPoleVertex = true;
            }
            else
            {
                *pPoleIndex = static_cast<uint32_t>(vertices.size());
                vertices.push_back(newPoleVertex);
            }
        }
    };

    fixPole(northPoleIndex);
    fixPole(southPoleIndex);

    // Reverse winding.
    for (auto it = indices.begin(); it != indices.end(); it += 3)
        std::swap(*it, *(it + 2));
    for (auto& it : vertices)
        it.Texture.x = (1.f - it.Texture.x);

    // Add the geosphere vertices and indices to the buffers.
    for (auto index : indices)
        m_indices.push_back(index);
    for (const auto& vertex : vertices)
        m_vertices.push_back(vertex);

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

/// <summary>
/// Builds a grid mesh in the xz-plane procedurally.
/// </summary>
/// <param name="gridWidth">Grid width. It determines the relative size of the grid.</param>
/// <param name="gridDepth">Grid depth. It determines the relative size of the grid.</param>
/// <param name="quadCountHoriz">The number of quads in the grid in the horizontal dimension (x-axis)</param>
/// <param name="quadCountDepth">The number of quads in the grid in the depth dimension (z-axis)</param>
void TextureMeshGenerator::CreateGrid(std::string const& name, float gridWidth, float gridDepth, uint32_t quadCountHoriz, uint32_t quadCountDepth)
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

    // Calculate increments for texture coordinates.
    float du = 1.0f / quadCountHoriz;
    float dv = 1.0f / quadCountDepth;

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
            m_vertices[info.BaseVertexLocation + k].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

            // Stretch the texture over grid.
            m_vertices[info.BaseVertexLocation + k].Texture.x = j * du;
            m_vertices[info.BaseVertexLocation + k].Texture.y = i * dv;

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

/// <summary>
/// Creates a pipe - a cylinder without caps.
/// </summary>
/// <param name="radius">The radius of the pipe</param>
/// <param name="height">The pipe's height</param>
/// <param name="sliceCount">The number of slices. A slice is one triangle in the top or the bottom of the pipe.</param>
/// <param name="stackCount">The number of stacks. A stack is one vertical segment of the pipe.</param>
void TextureMeshGenerator::CreatePipe(std::string const& name, float radius, float height, uint32_t sliceCount, uint32_t stackCount, bool createInterior)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    auto CreateVertices = [this, radius, height, sliceCount, stackCount](bool invertNormal)
    {
        float stackHeight = height / stackCount;

        // Calculate the angle of a single slice triangle.
        float theta = XM_2PI / sliceCount;

        // Generate vertices for each stack starting at the bottom stack. We use <= rater than <
        // because we want to generate vertices for the top of the last (top) stack.
        for (uint32_t i = 0; i <= stackCount; ++i)
        {
            // Calculate the y-coordinate of the i-th stack base (or the top of the last stack).
            float y = -0.5f * height + i * stackHeight;

            // Create vertices for the i-th stack. Note that we duplicate the first vertex as 
            // the last one by using <= rather than < 
            // This is necessary for correct texture rendering.
            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float c = cosf(j * theta);
                float s = sinf(j * theta);

                VertexPositionNormalTexture vertex;
                vertex.Position = XMFLOAT3(radius * c, y, radius * s);

                // Compute texture coordinates.
                vertex.Texture.x = (float)j / sliceCount;
                vertex.Texture.y = 1.0f - (float)i / stackCount;

                // Determine normal's sign.
                float sign = (invertNormal ? -1.f : 1.f);

                // Calculate vertex normal.
                XMVECTOR normal = sign * XMVector3Normalize(XMVectorSet(c, 0, s, 0));
                XMStoreFloat3(&vertex.Normal, normal);

                m_vertices.push_back(vertex);
            }
        }
    };

    CreateVertices(false); // exterior
    if (createInterior)
        CreateVertices(true); // interior

    auto CreateIndices = [this, sliceCount, stackCount](bool invertWinding)
    {
        // Increase the number of vertices per stack by one because we duplicated the first vertex.
        auto n = sliceCount + 1;

        // Calculate indices for each stack.
        for (uint32_t i = 0; i < stackCount; ++i)
        {
            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                auto A = i * n + j;
                auto B = (i + 1) * n + j;
                auto C = (i + 1) * n + j + 1;
                auto D = i * n + j + 1;

                if (invertWinding)
                {
                    // Each quad is composed of two triangles: ACB and ADC
                    m_indices.push_back(A);
                    m_indices.push_back(C);
                    m_indices.push_back(B);

                    m_indices.push_back(A);
                    m_indices.push_back(D);
                    m_indices.push_back(C);
                }
                else
                {
                    // Each quad is composed of two triangles: ABC and ACD
                    m_indices.push_back(A);
                    m_indices.push_back(B);
                    m_indices.push_back(C);

                    m_indices.push_back(A);
                    m_indices.push_back(C);
                    m_indices.push_back(D);
                }
            }
        }
    };

    CreateIndices(false); // exterior
    if (createInterior)
        CreateIndices(true); // interior

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

void TextureMeshGenerator::CreateQuad(std::string const& name)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    auto i = info.BaseVertexLocation;
    m_vertices.resize(i + 4);

    float l = 0.5f, n = 1.0f;

    m_vertices[i++] = { XMFLOAT3(-l,  0, -l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 1.0f) };
    m_vertices[i++] = { XMFLOAT3(-l,  0,  l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  0,  l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 0.0f) };
    m_vertices[i++] = { XMFLOAT3( l,  0, -l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 1.0f) };

    ASSERT(m_vertices.size() == i);

    std::vector<uint32_t> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    info.IndexCount = (uint32_t)indices.size();
    CopyIndices(indices, info.StartIndexLocation, info.IndexCount);

    m_meshes[name] = info;
}

void TextureMeshGenerator::CreateStar(std::string const& name, uint32_t armCount, float radiusShort, float radiusLong, float thickness)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    // Calculate the number of vertices and indices.
    const uint32_t VertexCount = 12 * armCount; // 12 vertices per arm
    const uint32_t IndexCount = 12 * armCount; // 12 indices per arm

    // Resize buffers.
    m_vertices.resize(m_vertices.size() + VertexCount);
    m_indices.resize(m_indices.size() + IndexCount);

    uint32_t v = info.BaseVertexLocation;
    uint32_t i = info.StartIndexLocation;

    // Prepare temporary values to calculate positions of the star's verticies and normals.
    float r1 = radiusShort, r2 = radiusLong, h = thickness / 2.0f; // short and long radius, and half of the star thickness (shorter variable names)
    float theta = 2.0f * XM_PI / (float)armCount; // an angle of the tip of the star's arm
    float x = tan(theta / 2.0f) * radiusShort; // a helper distance within the star's arm

    // Define geometry in the model space
    XMFLOAT3 A0, B0, C0, D0, E0;
    A0 = XMFLOAT3(0, -r2, 0);
    B0 = XMFLOAT3(x, -r1, 0);
    C0 = XMFLOAT3(-x, -r1, 0);
    D0 = XMFLOAT3(0, 0, -h);
    E0 = XMFLOAT3(0, 0, h);

    for (uint32_t n = 0; n < armCount; ++n)
    {
        float sint = sin((float)n * theta);
        float cost = cos((float)n * theta);

        XMFLOAT3 A, B, C, D, E;

        // rotate counterclockwise around the angle theta
        // x' = x*cos(a) - y*sin(a);
        // y' = x*sin(a) + y*cos(a);
        A.x = A0.x * cost - A0.y * sint;
        A.y = A0.x * sint + A0.y * cost;
        A.z = 0;

        B.x = B0.x * cost - B0.y * sint;
        B.y = B0.x * sint + B0.y * cost;
        B.z = 0;

        C.x = C0.x * cost - C0.y * sint;
        C.y = C0.x * sint + C0.y * cost;
        C.z = 0;

        // not rotated
        D = D0;
        E = E0;

        /*

                front
                  D
                 /|\       D(0,0)________ B(1,0)
                / | \           |        |
               /  |  \          |        |
            C /   |   \ B       |        |
              \   |   /         |________|
               \  |  /     C(0,1)         A(1,1)
                \ | /
                 \|/
                  A


        back
                  E
                 /|\       E(0,0)________ C(1,0)
                / | \           |        |
               /  |  \          |        |
            B /   |   \ C       |        |
              \   |   /         |________|
               \  |  /     B(0,1)         A(1,1)
                \ | /
                 \|/
                  A
        */

        XMFLOAT3 nv1; XMStoreFloat3(&nv1, Utilities::ComputeNormal(A, C, D));
        m_vertices[v++] = { A, nv1, XMFLOAT2(1.0f, 1.0f) };
        m_vertices[v++] = { C, nv1, XMFLOAT2(0.0f, 1.0f) };
        m_vertices[v++] = { D, nv1, XMFLOAT2(0.0f, 0.0f) };

        XMFLOAT3 nv2; XMStoreFloat3(&nv2, Utilities::ComputeNormal(A, D, B));
        m_vertices[v++] = { A, nv2, XMFLOAT2(1.0f, 1.0f) };
        m_vertices[v++] = { D, nv2, XMFLOAT2(0.0f, 0.0f) };
        m_vertices[v++] = { B, nv2, XMFLOAT2(1.0f, 0.0f) };

        XMFLOAT3 nv3; XMStoreFloat3(&nv3, Utilities::ComputeNormal(A, E, C));
        m_vertices[v++] = { A, nv3, XMFLOAT2(1.0f, 1.0f) };
        m_vertices[v++] = { E, nv3, XMFLOAT2(0.0f, 0.0f) };
        m_vertices[v++] = { C, nv3, XMFLOAT2(1.0f, 0.0f) };

        XMFLOAT3 nv4; XMStoreFloat3(&nv4, Utilities::ComputeNormal(A, B, E));
        m_vertices[v++] = { A, nv4, XMFLOAT2(1.0f, 1.0f) };
        m_vertices[v++] = { B, nv4, XMFLOAT2(0.0f, 1.0f) };
        m_vertices[v++] = { E, nv4, XMFLOAT2(0.0f, 0.0f) };

        for (int j = 0; j < 12; ++j)
        {
            m_indices[i] = i - info.StartIndexLocation;
            ++i;
        }
    }

    // Each arm has 12 vertices and indices.
    // The last index in the vertex/index buffer must be equal the total number of vertices/indices. 
    ASSERT(v - info.BaseVertexLocation == armCount * 12);
    ASSERT(i - info.StartIndexLocation == armCount * 12);

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

// Assumptions (for performance reasons): 
// - 250 - max characters per line
// - 10 - max tokens per line
static int const MAX_CHARS_PER_LINE = 250;
static int const MAX_TOKENS_PER_LINE = 10;

winrt::Windows::Foundation::IAsyncAction TextureMeshGenerator::CreateModelAsync(std::string name, winrt::hstring filename, bool hasTexture)
{
    ASSERT(m_meshes.find(name) == m_meshes.end());

    using namespace winrt;
    using namespace Windows::ApplicationModel;
    using namespace Windows::Storage;

    // Read lines from the input file.
    auto folder = Package::Current().InstalledLocation();
    StorageFile file{ co_await folder.GetFileAsync(filename) };
    auto lines = co_await FileIO::ReadLinesAsync(file);

    MeshInfo info;
    info.BaseVertexLocation = (uint32_t)m_vertices.size(); // initial vertex count
    info.StartIndexLocation = (uint32_t)m_indices.size(); // initial index count

    bool readVertices = false, readIndices = false;
    uint32_t baseVertex = info.BaseVertexLocation;
    uint32_t baseIndex = info.StartIndexLocation;

    std::vector<std::wstring> tokens;
    tokens.resize(MAX_TOKENS_PER_LINE);

    for (uint32_t i = 0; i < lines.Size(); ++i)
    {
        hstring line = lines.GetAt(i);
        GetTokes(line.c_str(), tokens);

        if (tokens[0] == L"VertexCount:")
        {
            int vertexCount = std::stoi(tokens[1]);
            m_vertices.resize((uint32_t)m_vertices.size() + vertexCount);
        }

        if (tokens[0] == L"TriangleCount:")
        {
            int triangleCount = std::stoi(tokens[1]);
            m_indices.resize((uint32_t)m_indices.size() + triangleCount * 3);
        }

        if (tokens[0] == L"VertexList") readVertices = true;
        if (tokens[0] == L"}" && readVertices) readVertices = false;
        if (tokens[0] == L"TriangleList") readIndices = true;
        if (tokens[0] == L"}" && readIndices) readIndices = false;

        if (readVertices)
        {
            if (tokens[0] != L"{" && tokens[0] != L"}" && tokens[0] != L"VertexList" && tokens[0] != L"")
            {
                float px = std::stof(tokens[0]);
                float py = std::stof(tokens[1]);
                float pz = std::stof(tokens[2]);
                float nx = std::stof(tokens[3]);
                float ny = std::stof(tokens[4]);
                float nz = std::stof(tokens[5]);

                if (hasTexture)
                {
                    float tx = std::stof(tokens[6]);
                    float ty = std::stof(tokens[7]);

                    m_vertices[baseVertex++] = { XMFLOAT3(px, py, pz), XMFLOAT3(nx, ny, nz), XMFLOAT2(tx, ty) };
                }
                else
                {
                    m_vertices[baseVertex++] = { XMFLOAT3(px, py, pz), XMFLOAT3(nx, ny, nz), XMFLOAT2(0.f, 0.f) }; // dummy texture coordinates just to fill the buffer
                }
            }
        }

        if (readIndices)
        {
            if (tokens[0] != L"{" && tokens[0] != L"}" && tokens[0] != L"TriangleList")
            {
                uint32_t x = std::stoi(tokens[0]);
                uint32_t y = std::stoi(tokens[1]);
                uint32_t z = std::stoi(tokens[2]);

                m_indices[baseIndex++] = x;
                m_indices[baseIndex++] = y;
                m_indices[baseIndex++] = z;
            }
        }
    }

    info.IndexCount = (uint32_t)m_indices.size() - info.StartIndexLocation;

    m_meshes[name] = info;
}

// Splits a string into tokens.
void TextureMeshGenerator::GetTokes(const wchar_t* ps, std::vector<std::wstring>& tokens)
{
    static wchar_t p[MAX_CHARS_PER_LINE];
    unsigned l = 0;
    int k = 0;

    for (unsigned j = 0; ps[j] != 0; ++j)
    {
        if (ps[j] != ' ')
        {
            if (ps[j] != '\t')
            {
                p[k] = ps[j];
                ++k;
            }
        }
        else
        {
            p[k] = 0;
            tokens[l] = p;
            ++l;
            k = 0;
        }
    }

    p[k] = 0;
    tokens[l] = p;
    l = 0;
}

void TextureMeshGenerator::CreateBuffers()
{
    // Create an immutable vertex buffer and load data.
    m_vertexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_VERTEX_BUFFER,
            (uint32_t)m_vertices.size() * sizeof(VertexPositionNormalTexture),
            m_vertices.data()));

    // Create an immutable index buffer and load indices to the buffer.
    m_indexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_INDEX_BUFFER,
            (uint32_t)m_indices.size() * sizeof(uint32_t),
            m_indices.data()));
}

void TextureMeshGenerator::SetBuffers()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Each vertex is one instance of the VertexPositionNormal struct.
    UINT stride = sizeof(VertexPositionNormalTexture);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    // Each index is one 32-bit unsigned integer.
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
}

void TextureMeshGenerator::DrawMesh(std::string const& name)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    const auto& info = m_meshes[name];

    // Draw one object at a time as each object may have a different world matrix.
    context->DrawIndexed(info.IndexCount, info.StartIndexLocation, info.BaseVertexLocation);
}

void TextureMeshGenerator::Clear()
{
    // Clear collections.
    m_vertices.clear();
    m_indices.clear();
    m_meshes.clear();

    // Release buffers.
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}
