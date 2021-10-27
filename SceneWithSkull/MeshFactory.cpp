#include "pch.h"

#include "MeshFactory.h"

using namespace DirectX;

MeshFactory::MeshFactory(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
}

void MeshFactory::AddCube()
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

void MeshFactory::AddPyramid()
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
