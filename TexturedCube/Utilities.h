#pragma once

// Reads data from a binary file.
static winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> ReadDataAsync(winrt::hstring const& filename)
{
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::Storage;

    auto folder = Package::Current().InstalledLocation();
    StorageFile file{ co_await folder.GetFileAsync(filename) };
    co_return co_await FileIO::ReadBufferAsync(file);
}

static ID3D11Buffer* CreateImmutableVertexBuffer(ID3D11Device3* device, unsigned int byteWidth, void const* data)
{
    ID3D11Buffer* pBuffer;

    // Create the vertex buffer description.
    D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
    vertexBufferDesc.ByteWidth = byteWidth;                     // the size of the buffer in bytes
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;             // the contents of the buffer will not change after creation
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;                        // CPU does not require read or write access to the buffer after the buffer has been created
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;                   // used with structured buffers; 0 for the vertex buffers

    // Create data structure to initialize the vertex buffer.
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = data;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    // Create the vertex buffer and load data.
    winrt::check_hresult(
        device->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            &pBuffer));

    return pBuffer;
}
