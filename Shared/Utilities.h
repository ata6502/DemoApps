#pragma once

void DebugTrace(const wchar_t* format, ...);

class Utilities
{
public:
    // Reads data from a binary file.
    static winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> ReadDataAsync(winrt::hstring const& filename);

    // Create an immutable buffer.
    static ID3D11Buffer* CreateImmutableBuffer(ID3D11Device3* device, D3D11_BIND_FLAG bufferType, unsigned int byteWidth, void const* data);
};
