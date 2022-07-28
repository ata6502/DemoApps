#include "pch.h"

#include "BoxMesh.h"
#include "Utilities.h"
#include "VertexStructures.h"

using namespace DirectX;

BoxMesh::BoxMesh(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_indexCount(0)
{
}

void BoxMesh::CreateWithTexture()
{
    ReleaseResources();

    auto device{ m_deviceResources->GetD3DDevice() };

    // Create box vertices. 
    float l = 0.5f, n = 1.0f;
    static const VertexPositionTexture boxVertices[] =
    {
        // front face
        { XMFLOAT3(-l, -l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-l, +l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(+l, +l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(+l, -l, -l), XMFLOAT3(0, 0, -n), XMFLOAT2(1.0f, 1.0f) },

        // back face
        { XMFLOAT3(-l, -l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(+l, -l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(+l, +l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-l, +l, +l), XMFLOAT3(0, 0, n), XMFLOAT2(1.0f, 0.0f) },

        // top face
        { XMFLOAT3(-l, +l, -l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-l, +l, +l), XMFLOAT3(0, n, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(+l, +l, +l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(+l, +l, -l), XMFLOAT3(0, n, 0), XMFLOAT2(1.0f, 1.0f) },

        // bottom face
        { XMFLOAT3(-l, -l, -l), XMFLOAT3(0, -n, 0), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(+l, -l, -l), XMFLOAT3(0, -n, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(+l, -l, +l), XMFLOAT3(0, -n, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-l, -l, +l), XMFLOAT3(0, -n, 0), XMFLOAT2(1.0f, 0.0f) },

        // left face
        { XMFLOAT3(-l, -l, +l), XMFLOAT3(-n, 0, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-l, +l, +l), XMFLOAT3(-n, 0, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-l, +l, -l), XMFLOAT3(-n, 0, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-l, -l, -l), XMFLOAT3(-n, 0, 0), XMFLOAT2(1.0f, 1.0f) },

        // right face
        { XMFLOAT3(+l, -l, -l), XMFLOAT3(n, 0, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(+l, +l, -l), XMFLOAT3(n, 0, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(+l, +l, +l), XMFLOAT3(n, 0, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(+l, -l, +l), XMFLOAT3(n, 0, 0), XMFLOAT2(1.0f, 1.0f) }
    };

    // Create an immutable vertex buffer.
    m_vertexBuffer.attach(Utilities::CreateImmutableBuffer(device, D3D11_BIND_VERTEX_BUFFER, sizeof(boxVertices), &boxVertices));

    // Create cube indices in the left-handed coordinate system.
    static const uint32_t boxIndices[] =
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
        20, 22, 23
    };

    // Keep the number of indices.
    m_indexCount = ARRAYSIZE(boxIndices);

    // Create index buffer and load indices to the buffer.
    m_indexBuffer.attach(
        Utilities::CreateImmutableBuffer(
            m_deviceResources->GetD3DDevice(),
            D3D11_BIND_INDEX_BUFFER,
            sizeof(boxIndices),
            &boxIndices));
}

void BoxMesh::SetBuffers(unsigned int stride)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    // Each index is one 32-bit unsigned integer (short).
    context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
}

void BoxMesh::Draw()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    context->DrawIndexed(m_indexCount, 0, 0);
}

void BoxMesh::ReleaseResources()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}
