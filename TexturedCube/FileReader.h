#pragma once

#include "FileReaderStructures.h"

class FileReader
{
public:
    static winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> ReadDataAsync(winrt::hstring const& filename);
    static winrt::Windows::Foundation::IAsyncAction LoadTextureAsync(ID3D11Device3* device, winrt::hstring filename, ID3D11ShaderResourceView** textureView);

private:
    static void CreateTexture(
        ID3D11Device3* device, 
        const byte* data, 
        uint32_t dataSize, 
        ID3D11ShaderResourceView** textureView);

    static void CreateDDSTextureFromMemory(
        ID3D11Device* d3dDevice,
        const uint8_t* ddsData,
        size_t ddsDataSize,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView,
        size_t  maxsize = 0,
        D2D1_ALPHA_MODE* alphaMode = nullptr);

    static void CreateDDSTextureFromMemoryEx(
        ID3D11Device* d3dDevice,
        const uint8_t* ddsData,
        size_t ddsDataSize,
        size_t maxsize,
        D3D11_USAGE usage,
        unsigned int bindFlags,
        unsigned int cpuAccessFlags,
        unsigned int miscFlags,
        bool forceSRGB,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView,
        D2D1_ALPHA_MODE* alphaMode = nullptr);

    static void CreateTextureFromDDS(
        ID3D11Device* d3dDevice,
        const DDS_HEADER* header,
        const byte* bitData,
        size_t bitSize,
        size_t maxsize,
        D3D11_USAGE usage,
        unsigned int bindFlags,
        unsigned int cpuAccessFlags,
        unsigned int miscFlags,
        bool forceSRGB,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView);

    static D2D1_ALPHA_MODE GetAlphaMode(const DDS_HEADER* header);

    static size_t BitsPerPixel(DXGI_FORMAT fmt);

    static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf);

    static void FillInitData(
        size_t width,
        size_t height,
        size_t depth,
        size_t mipCount,
        size_t arraySize,
        DXGI_FORMAT format,
        size_t maxsize,
        size_t bitSize,
        const byte* bitData,
        size_t& twidth,
        size_t& theight,
        size_t& tdepth,
        size_t& skipMip,
        D3D11_SUBRESOURCE_DATA* initData);

    static HRESULT CreateD3DResources(
        ID3D11Device* d3dDevice,
        uint32_t resDim,
        size_t width,
        size_t height,
        size_t depth,
        size_t mipCount,
        size_t arraySize,
        DXGI_FORMAT format,
        D3D11_USAGE usage,
        unsigned int bindFlags,
        unsigned int cpuAccessFlags,
        unsigned int miscFlags,
        bool forceSRGB,
        bool isCubeMap,
        D3D11_SUBRESOURCE_DATA* initData,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView);

    static void GetSurfaceInfo(
        size_t width,
        size_t height,
        DXGI_FORMAT fmt,
        size_t* outNumBytes,
        size_t* outRowBytes,
        size_t* outNumRows);

    static DXGI_FORMAT MakeSRGB(DXGI_FORMAT format);
};

