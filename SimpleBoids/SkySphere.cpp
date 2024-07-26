#include "pch.h"

#include "SkySphere.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

SkySphere::SkySphere(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_vertexBuffer(nullptr),
    m_indexBuffer(nullptr),
    m_indexCount(0)
{
}

/// <summary>
/// Based on [Luna]
/// Creates a sphere using an approach similar to creating a cylinder.
/// We use trigonometric functions to calculate the radius per stack.
/// Note that the triangles of the sphere do not have equal areas.
/// </summary>
/// <param name="radius">The sphere's radius</param>
/// <param name="sliceCount">The number of slices</param>
/// <param name="stackCount">The number of stacks</param>
void SkySphere::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
    // Create the sphere's poles.
    VertexPosition topVertex{ { 0.0f, radius, 0.0f } };
    VertexPosition bottomVertex{ { 0.0f, -radius, 0.0f } };

    std::vector<VertexPosition> vertices;
    std::vector<uint32_t> indices;

    // Add the north pole as the first vertex.
    vertices.push_back(topVertex);

    float phiStep = XM_PI / stackCount;
    float thetaStep = XM_2PI / sliceCount;

    // Compute vertices for each stack.
    for (uint32_t i = 0; i <= stackCount; ++i)
    {
        float phi = (i+1) * phiStep;

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            VertexPosition v;

            // Convert spherical coordinates to Cartesian coordinates.
            v.Position.x = radius * sinf(phi) * cosf(theta);
            v.Position.y = radius * cosf(phi);
            v.Position.z = radius * sinf(phi) * sinf(theta);

            vertices.push_back(v);
        }
    }

    // Add the south pole as the last vertex.
    vertices.push_back(bottomVertex);

    // Compute indices for the top stack which contains the north pole.
    for (uint32_t i = 1; i <= sliceCount; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    // Compute indices for inner stacks.
    auto baseIndex = 1; // skip the north pole vertex
    auto n = sliceCount + 1; // the number of vertices in a stack
    for (uint32_t i = 0; i < stackCount - 2; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            indices.push_back(baseIndex + i * n + j);
            indices.push_back(baseIndex + i * n + j + 1);
            indices.push_back(baseIndex + (i + 1) * n + j);

            indices.push_back(baseIndex + (i + 1) * n + j);
            indices.push_back(baseIndex + i * n + j + 1);
            indices.push_back(baseIndex + (i + 1) * n + j + 1);
        }
    }

    // Compute indices for the bottom stack which contains the south pole.
    uint32_t southPoleIndex = (uint32_t)vertices.size() - 1;

    // Offset the indices to the index of the first vertex in the last stack.
    baseIndex = southPoleIndex - n;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        indices.push_back(southPoleIndex);
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + 1);
    }

    m_indexCount = (uint32_t)indices.size();

    // Create an immutable vertex buffer and load data.
    m_vertexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_VERTEX_BUFFER,
            (uint32_t)vertices.size() * sizeof(VertexPosition),
            vertices.data()));

    // Create an immutable index buffer and load indices to the buffer.
    m_indexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_INDEX_BUFFER,
            (uint32_t)indices.size() * sizeof(uint32_t),
            indices.data()));
}

void SkySphere::SetBuffers()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Each vertex is one instance of the VertexPosition struct.
    UINT stride = sizeof(VertexPosition);
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    // Each index is one 32-bit unsigned integer.
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
}

void SkySphere::DrawSphere()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };
    context->DrawIndexed(m_indexCount, 0, 0);
}

void SkySphere::ReleaseDeviceDependentResources()
{
    // Release buffers.
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}

