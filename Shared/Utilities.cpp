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
ID3D11Buffer* Utilities::CreateImmutableBuffer(ID3D11Device3* device, D3D11_BIND_FLAG bufferType, unsigned int byteWidth, void const* data)
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

#if defined(_DEBUG)
// Examples: 
// DebugTrace(L"num = %4.2f\n", num);
// DebugTrace(L"%s\n", str.c_str());
void DebugTrace(const wchar_t* format, ...)
{
    // Generate the message string.
    va_list args;
    va_start(args, format); // initialize the argument list
    wchar_t buffer[1024];
    ASSERT(_vsnwprintf_s(buffer, _countof(buffer) - 1, format, args) != -1);
    va_end(args);

    OutputDebugStringW(buffer); // this is a Windows function
}
#endif