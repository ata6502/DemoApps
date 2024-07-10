#include "pch.h"
#include "Utilities.h"

// Reads data from a binary file.
winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> Utilities::ReadDataAsync(winrt::hstring const& filename)
{
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::Storage;

    auto folder = Package::Current().InstalledLocation();
    StorageFile file{ co_await folder.GetFileAsync(filename) };
    co_return co_await FileIO::ReadBufferAsync(file);
}

// Create an immutable buffer.
ID3D11Buffer* Utilities::CreateImmutableBuffer(ID3D11Device3* device, D3D11_BIND_FLAG bufferType, uint32_t byteWidth, void const* data)
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

DirectX::XMVECTOR Utilities::ComputeNormal(DirectX::XMFLOAT3 const& p0, DirectX::XMFLOAT3 const& p1, DirectX::XMFLOAT3 const& p2)
{
    using namespace DirectX;

    XMVECTOR u = XMLoadFloat3(&p1) - XMLoadFloat3(&p0);
    XMVECTOR v = XMLoadFloat3(&p2) - XMLoadFloat3(&p0);
    return XMVector3Normalize(XMVector3Cross(u, v));
}

DirectX::XMMATRIX Utilities::CalculateInverseTranspose(DirectX::CXMMATRIX M)
{
    using namespace DirectX;

    // Zero out the translation row in the input matrix because 
    // we apply an inverse-transpose matrix just to normals. 
    // We don't want the inverse-transpose of the translation.
    XMMATRIX A = M;
    A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMVECTOR det = XMMatrixDeterminant(A);
    return XMMatrixTranspose(XMMatrixInverse(&det, A));
}