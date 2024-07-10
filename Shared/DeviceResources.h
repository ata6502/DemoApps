#pragma once

namespace DX
{
    // An interface for a class that owns DeviceResources to be notified when the device is lost or created.
    interface IDeviceNotify
    {
        virtual void OnDeviceLost() = 0;
        virtual void OnDeviceRestored() = 0;

    protected:
        ~IDeviceNotify() = default;
    };

	// Creates the Direct3D device and the device context as well as the swap chain. It also manages the device lost event. 
    class DeviceResources
    {
    public:
        DeviceResources(
            DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
        ~DeviceResources() = default;

        DeviceResources(DeviceResources&&) = default;
        DeviceResources& operator= (DeviceResources&&) = default;

        DeviceResources(DeviceResources const&) = delete;
        DeviceResources& operator= (DeviceResources const&) = delete;

        void SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel); // used with XAML apps

        void SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize);
        void SetCompositionScale(float compositionScaleX, float compositionScaleY); // used only with XAML apps
        void SetCurrentOrientation(winrt::Windows::Graphics::Display::DisplayOrientations currentOrientation);
        void SetDpi(float dpi);

        void Present();
        void ValidateDevice();
        void Trim();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify);

        // D3D Accessors
        ID3D11Device3*                          GetD3DDevice() const { return m_d3dDevice.get(); }
        ID3D11DeviceContext3*                   GetD3DDeviceContext() const { return m_d3dContext.get(); }
        ID3D11RenderTargetView*                 GetRenderTargetView() const { return m_d3dRenderTargetView.get(); } // returns the back-buffer render target view
        ID3D11DepthStencilView*                 GetDepthStencilView() const { return m_d3dDepthStencilView.get(); }
        IDXGISwapChain1*                        GetSwapChain() const { return m_swapChain.get(); }
        D3D11_VIEWPORT                          GetScreenViewport() const { return m_screenViewport; }
        D3D_FEATURE_LEVEL                       GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
        ID3D11Texture2D*                        GetRenderTarget() const { return m_renderTarget.get(); }
        DirectX::XMMATRIX                       GetOrientationTransform3D() const { return XMLoadFloat4x4(&m_orientationTransform3D); }

        // D2D Accessors.
        ID2D1Factory3*                          GetD2DFactory() const { return m_d2dFactory.get(); }
        ID2D1Device2*                           GetD2DDevice() const { return m_d2dDevice.get(); }
        ID2D1DeviceContext2*                    GetD2DDeviceContext() const { return m_d2dContext.get(); }
        ID2D1Bitmap1*                           GetD2DTargetBitmap() const { return m_d2dTargetBitmap.get(); }
        IDWriteFactory3*                        GetDWriteFactory() const { return m_dwriteFactory.get(); }
        IWICImagingFactory2*                    GetWicImagingFactory() const { return m_wicFactory.get(); }
        D2D1::Matrix3x2F                        GetOrientationTransform2D() const { return m_orientationTransform2D; }

        // Device Accessors.
        winrt::Windows::Foundation::Size        GetOutputSize() const { return m_outputSize; }
        winrt::Windows::Foundation::Size        GetLogicalSize() const { return m_logicalSize; }
        winrt::Windows::Foundation::Size        GetRenderTargetSize() const { return m_d3dRenderTargetSize; }
        float                                   GetDpi() const { return m_dpi; }

    private:
        // Direct3D and DXGI objects.
        winrt::com_ptr<ID3D11Device3>           m_d3dDevice;
        winrt::com_ptr<ID3D11DeviceContext3>    m_d3dContext;
        winrt::com_ptr<IDXGISwapChain1>         m_swapChain;        // there is a new version: IDXGISwapChain3

        // Direct3D rendering objects required for 3D.
        winrt::com_ptr<ID3D11RenderTargetView>  m_d3dRenderTargetView;
        winrt::com_ptr<ID3D11DepthStencilView>  m_d3dDepthStencilView;
        winrt::com_ptr<ID3D11Texture2D>         m_renderTarget;     // the back buffer
        winrt::com_ptr<ID3D11Texture2D>         m_depthStencil;
        D3D11_VIEWPORT                          m_screenViewport;

        // Direct2D drawing components.
        winrt::com_ptr<ID2D1Factory3>           m_d2dFactory;
        winrt::com_ptr<ID2D1Device2>            m_d2dDevice;
        winrt::com_ptr<ID2D1DeviceContext2>     m_d2dContext;
        winrt::com_ptr<ID2D1Bitmap1>            m_d2dTargetBitmap;

        // DirectWrite drawing components.
        winrt::com_ptr<IDWriteFactory3>         m_dwriteFactory;    // DWriteCreateFactory is located in dwrite.lib
        winrt::com_ptr<IWICImagingFactory2>     m_wicFactory;       // IWICImagingFactory2 is located in windowscodecs.lib

        // A cached reference to the UI control. XAML apps only.
        winrt::Windows::UI::Xaml::Controls::SwapChainPanel m_swapChainPanel;

        // Cached device properties.
        D3D_FEATURE_LEVEL                       m_d3dFeatureLevel;
        winrt::Windows::Foundation::Size        m_d3dRenderTargetSize;
        winrt::Windows::Foundation::Size        m_outputSize;       // the size of the render target in pixels
        winrt::Windows::Foundation::Size        m_logicalSize;      // the size of the render target in DIPs (Device Independent Units)
        float                                   m_compositionScaleX; // used only with XAML apps
        float                                   m_compositionScaleY; // used only with XAML apps
        winrt::Windows::Graphics::Display::DisplayOrientations m_nativeOrientation;
        winrt::Windows::Graphics::Display::DisplayOrientations m_currentOrientation;
        float                                   m_dpi;
        DXGI_FORMAT                             m_backBufferFormat;
        DXGI_FORMAT                             m_depthBufferFormat;

        // Transforms used for display orientation.
        D2D1::Matrix3x2F                        m_orientationTransform2D;
        DirectX::XMFLOAT4X4                     m_orientationTransform3D;

        // Private helper methods.
        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void HandleDeviceLost();
        DXGI_MODE_ROTATION ComputeDisplayRotation();

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        IDeviceNotify* m_deviceNotify;
    };
}
