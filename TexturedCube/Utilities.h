#pragma once


// Create an immutable buffer.
static ID3D11Buffer* CreateImmutableBuffer(ID3D11Device3* device, D3D11_BIND_FLAG bufferType, unsigned int byteWidth, void const* data)
{
    ASSERT(bufferType == D3D11_BIND_VERTEX_BUFFER || bufferType == D3D11_BIND_INDEX_BUFFER);

    ID3D11Buffer* pBuffer;

    // Create the buffer description.
    D3D11_BUFFER_DESC bufferDesc = { 0 };
    bufferDesc.ByteWidth = byteWidth;                     // the size of the buffer in bytes
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;             // the contents of the buffer will not change after creation
    bufferDesc.BindFlags = bufferType;
    bufferDesc.CPUAccessFlags = 0;                        // CPU does not require read or write access to the buffer after the buffer has been created
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;                   // used with structured buffers

    // Create data structure to initialize the buffer.
    D3D11_SUBRESOURCE_DATA bufferData = { 0 };
    bufferData.pSysMem = data;
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    // Create the buffer and load data.
    winrt::check_hresult(
        device->CreateBuffer(
            &bufferDesc,
            &bufferData,
            &pBuffer));

    return pBuffer;
}
