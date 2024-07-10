#pragma once

class Utilities
{
public:
    // Reads data from a binary file.
    static winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> ReadDataAsync(winrt::hstring const& filename);

    // Creates an immutable buffer.
    static ID3D11Buffer* CreateImmutableBuffer(ID3D11Device3* device, D3D11_BIND_FLAG bufferType, uint32_t byteWidth, void const* data);

    // Computes a face normal of a triangle P1P2P3.
    static DirectX::XMVECTOR ComputeNormal(DirectX::XMFLOAT3 const& p0, DirectX::XMFLOAT3 const& p1, DirectX::XMFLOAT3 const& p2);

    // Calculates the inverse transpose of a given matrix.
    static DirectX::XMMATRIX CalculateInverseTranspose(DirectX::CXMMATRIX M);
};
