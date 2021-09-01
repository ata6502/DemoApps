#pragma once

namespace DX
{
    // An interface for a class that owns DeviceResources to be notified when the device is lost or created.
    interface IDeviceNotify
    {
        virtual void OnDeviceLost() = 0;
        virtual void OnDeviceRestored() = 0;
    };

    class DeviceResources
    {
    public:
        DeviceResources();

        void SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel);
        void SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize);
        void SetCompositionScale(float compositionScaleX, float compositionScaleY);
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
        void ValidateDevice();
        void Trim();
        void Present();

        winrt::Windows::Foundation::Size        GetOutputSize() const { return m_outputSize; }
        winrt::Windows::Foundation::Size        GetLogicalSize() const { return m_logicalSize; }

        // D3D Accessors
        ID3D11Device3*                          GetD3DDevice() const { return m_d3dDevice.get(); }
        ID3D11DeviceContext3*                   GetD3DDeviceContext() const { return m_d3dContext.get(); }
        ID3D11RenderTargetView*                 GetBackBufferRenderTargetView() const { return m_d3dRenderTargetView.get(); }
        ID3D11DepthStencilView*                 GetDepthStencilView() const { return m_d3dDepthStencilView.get(); }

    private:
        void CreateDevice();
        void CreateWindowSizeDependentResources();
        void HandleDeviceLost();

        // Direct3D objects.
        winrt::com_ptr<ID3D11Device3>                   m_d3dDevice;
        winrt::com_ptr<ID3D11DeviceContext3>            m_d3dContext;
        winrt::com_ptr<ID3D11RenderTargetView>          m_d3dRenderTargetView;
        winrt::com_ptr<ID3D11DepthStencilView>          m_d3dDepthStencilView;
        D3D11_VIEWPORT                                  m_screenViewport;

        // DXGI objects.
        winrt::com_ptr<IDXGISwapChain1>                 m_swapChain;

        // Cached reference to the SwapChainPanel UI control.
        winrt::Windows::UI::Xaml::Controls::SwapChainPanel m_swapChainPanel;

        // The size of the render target as well as the swap chain.
        winrt::Windows::Foundation::Size                m_outputSize;  // the size of the render target in pixels
        winrt::Windows::Foundation::Size                m_logicalSize; // the size of the render target in DIPs (Device Independent Units)
        float                                           m_compositionScaleX;
        float                                           m_compositionScaleY;

        IDeviceNotify*                                  m_deviceNotify;
    };
}
