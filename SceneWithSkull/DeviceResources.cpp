#include "pch.h"

#include "DeviceResources.h"

using namespace winrt;
using namespace Windows::Foundation;

// Forward declarations.
inline bool SdkLayersAvailable();

DX::DeviceResources::DeviceResources() :
    m_outputSize(),
    m_logicalSize(),
    m_compositionScaleX(1.0f),
    m_compositionScaleY(1.0f),
    m_screenViewport()
{
    CreateDevice();
}

/// <summary>
/// Creates Direct3D device and device context. 
/// </summary>
void DX::DeviceResources::CreateDevice()
{
    // Describes parameters that are used to create a device using D3D11_CREATE_DEVICE_FLAG enum.
    uint32_t creationFlags = 0;

#if defined(_DEBUG)
    if (SdkLayersAvailable())
    {
        // If the project is in a debug build, enable debugging via SDK Layers.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif

    // Define an array of feature levels this app supports. The ordering in 
    // the array is important. 
    // All applications are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // Create the Direct3D device and the context.
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL d3dFeatureLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // use the hardware graphics driver
        0,                          // always 0 except for D3D_DRIVER_TYPE_SOFTWARE
        creationFlags,
        featureLevels,              // the list of feature levels this app can support
        ARRAYSIZE(featureLevels),   // the size of the feature level list
        D3D11_SDK_VERSION,          // always D3D11_SDK_VERSION for WinRT apps
        device.put(),               // the Direct3D device created
        &d3dFeatureLevel,           // the feature level of device created
        context.put());             // the device immediate context

    if (FAILED(hr))
    {
        // If the initialization failed, fall back to the WARP device.
        // http://go.microsoft.com/fwlink/?LinkId=286690
        winrt::check_hresult(
            D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP, // create a WARP device instead of a hardware device
                0,
                creationFlags,
                featureLevels,
                ARRAYSIZE(featureLevels),
                D3D11_SDK_VERSION,
                device.put(),
                &d3dFeatureLevel,
                context.put()));
    }

    // Store pointers to the Direct3D 11.3 API device and the immediate context.
    m_d3dDevice = device.as<ID3D11Device3>();
    m_d3dContext = context.as<ID3D11DeviceContext3>();
}

/// <summary>
/// Creates resources that depend on the window size. They need to be
/// recreated every time the window size changes.
/// </summary>
void DX::DeviceResources::CreateWindowSizeDependentResources()
{
    // Clear the previous window size-specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    m_d3dRenderTargetView = nullptr;
    m_d3dDepthStencilView = nullptr;
    m_d3dContext->Flush();

    // Calculate the swap chain and render target size in pixels.
    // Also, prevent zero size content from being created.
    m_outputSize.Width = std::max(m_logicalSize.Width * m_compositionScaleX, 1.0f);
    m_outputSize.Height = std::max(m_logicalSize.Height * m_compositionScaleY, 1.0f);

    if (m_swapChain == nullptr)
    {
        // Create a swap chain using the same adapter as the Direct3D device.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

        swapChainDesc.Width = static_cast<uint32_t>(m_outputSize.Width);    // match the size of the window
        swapChainDesc.Height = static_cast<uint32_t>(m_outputSize.Height);
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;                  // the most common swap chain format
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;                                 // don't use multi-sampling
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;                                      // use double-buffering
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;        // Windows Store apps must use FLIP SwapEffects
        swapChainDesc.Flags = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        // Get the DXGI device.
        winrt::com_ptr<IDXGIDevice3> dxgiDevice = m_d3dDevice.as<IDXGIDevice3>();

        // Get the DXGI adapter.
        winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
        winrt::check_hresult(
            dxgiDevice->GetAdapter(dxgiAdapter.put()));

        // Get the DXGI factory.
        winrt::com_ptr<IDXGIFactory3> dxgiFactory =
            winrt::capture<IDXGIFactory3>(dxgiAdapter, &IDXGIAdapter::GetParent);

        // When using XAML interop, the swap chain must be created for composition.
        winrt::check_hresult(
            dxgiFactory->CreateSwapChainForComposition(
                m_d3dDevice.get(),
                &swapChainDesc,
                nullptr,
                m_swapChain.put()));

        // Associate swap chain with SwapChainPanel. UI changes need 
        // to be dispatched back to the UI thread.
        m_swapChainPanel.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::High, [=]()
        {
            // Get an ISwapChainPanelNative from the SwapChainPanel UI control.
            auto panelNative{ m_swapChainPanel.as<ISwapChainPanelNative>() };

            // Associate the swap chain with the SwapChainPanel UI control.
            winrt::check_hresult(
                panelNative->SetSwapChain(m_swapChain.get()));
        });

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
        // ensures that the application will only render after each VSync, minimizing power consumption.
        winrt::check_hresult(
            dxgiDevice->SetMaximumFrameLatency(1));
    }
    else
    {
        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(
            2, // double-buffered swap chain
            static_cast<uint32_t>(m_outputSize.Width),
            static_cast<uint32_t>(m_outputSize.Height),
            DXGI_FORMAT_B8G8R8A8_UNORM,
            0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            winrt::check_hresult(hr);
        }
    }

    // Setup inverse scale on the swap chain.
    DXGI_MATRIX_3X2_F inverseScale = { 0 };
    inverseScale._11 = 1.0f / m_compositionScaleX;
    inverseScale._22 = 1.0f / m_compositionScaleY;
    auto swapChain{ m_swapChain.as<IDXGISwapChain2>() }; // cast to IDXGISwapChain2 to call SetMatrixTransform
    winrt::check_hresult(
        swapChain->SetMatrixTransform(&inverseScale));

    // Create a render target view of the swap chain back buffer.
    winrt::com_ptr<ID3D11Texture2D> backBuffer =
        winrt::capture<ID3D11Texture2D>(m_swapChain, &IDXGISwapChain1::GetBuffer, 0);

    winrt::check_hresult(
        m_d3dDevice->CreateRenderTargetView(
            backBuffer.get(),
            nullptr,
            m_d3dRenderTargetView.put()));

    // Create a depth stencil view.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        static_cast<uint32_t>(m_outputSize.Width),
        static_cast<uint32_t>(m_outputSize.Height),
        1, // use one texture for this depth-stencil view
        1, // use a single mipmap level
        D3D11_BIND_DEPTH_STENCIL);

    winrt::com_ptr<ID3D11Texture2D> depthStencil;
    winrt::check_hresult(
        m_d3dDevice->CreateTexture2D(
            &depthStencilDesc,
            nullptr,
            depthStencil.put()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    winrt::check_hresult(
        m_d3dDevice->CreateDepthStencilView(
            depthStencil.get(),
            &depthStencilViewDesc,
            m_d3dDepthStencilView.put()));

    // Set the 3D rendering viewport to target the entire window.
    m_screenViewport = CD3D11_VIEWPORT(
        0.0f,
        0.0f,
        m_outputSize.Width,
        m_outputSize.Height
    );

    m_d3dContext->RSSetViewports(1, &m_screenViewport);
}

/// <summary>
/// Caches a reference to a SwapChainPanel UI control as well as some 
/// properties related to the size of the panel.
/// </summary>
void DX::DeviceResources::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel)
{
    m_swapChainPanel = panel;
    m_logicalSize = Size(static_cast<float>(panel.ActualWidth()), static_cast<float>(panel.ActualHeight()));
    m_compositionScaleX = panel.CompositionScaleX();
    m_compositionScaleY = panel.CompositionScaleY();

    CreateWindowSizeDependentResources();
}

/// <summary>
/// Called when the size of the swap chain panel is changed. 
/// </summary>
void DX::DeviceResources::SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize)
{
    if (m_logicalSize != logicalSize)
    {
        m_logicalSize = logicalSize;
        CreateWindowSizeDependentResources();
    }
}

/// <summary>
/// Called in the OnCompositionScaleChanged event handler. 
/// </summary>
void DX::DeviceResources::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
    if (m_compositionScaleX != compositionScaleX ||
        m_compositionScaleY != compositionScaleY)
    {
        m_compositionScaleX = compositionScaleX;
        m_compositionScaleY = compositionScaleY;
        CreateWindowSizeDependentResources();
    }
}

/// <summary>
/// Presents the contents of the swap chain to the screen. 
/// </summary>
void DX::DeviceResources::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be modified.
    m_d3dContext->DiscardView1(m_d3dRenderTargetView.get(), nullptr, 0);

    // Discard the contents of the depth stencil.
    m_d3dContext->DiscardView1(m_d3dDepthStencilView.get(), nullptr, 0);

    // If the device was removed either by a disconnection or a driver upgrade, we 
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        HandleDeviceLost();
    }
    else
    {
        winrt::check_hresult(hr);
    }
}

/// <summary>
/// Called in the OnDisplayContentsInvalidated event handler.
/// </summary>
void DX::DeviceResources::ValidateDevice()
{
    // The D3D Device is no longer valid if the default adapter changed since the device
    // was created or if the device has been removed.

    // First, get the information for the default adapter from when the device was created.

    auto dxgiDevice{ m_d3dDevice.as<IDXGIDevice3>() };

    winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
    winrt::check_hresult(dxgiDevice->GetAdapter(dxgiAdapter.put()));

    winrt::com_ptr<IDXGIFactory3> dxgiFactory;
    dxgiFactory.capture(dxgiAdapter, &IDXGIAdapter::GetParent);

    winrt::com_ptr<IDXGIAdapter1> previousDefaultAdapter;
    winrt::check_hresult(dxgiFactory->EnumAdapters1(0, previousDefaultAdapter.put()));

    DXGI_ADAPTER_DESC1 previousDesc;
    winrt::check_hresult(previousDefaultAdapter->GetDesc1(&previousDesc));

    // Next, get the information for the current default adapter.

    winrt::com_ptr<IDXGIFactory3> currentFactory;
    currentFactory.capture(&CreateDXGIFactory1);

    winrt::com_ptr<IDXGIAdapter1> currentDefaultAdapter;
    winrt::check_hresult(currentFactory->EnumAdapters1(0, currentDefaultAdapter.put()));

    DXGI_ADAPTER_DESC currentDesc;
    winrt::check_hresult(currentDefaultAdapter->GetDesc(&currentDesc));

    // If the adapter LUIDs don't match, or if the device reports that it has been removed,
    // a new D3D device must be created.

    if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
        previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
        FAILED(m_d3dDevice->GetDeviceRemovedReason()))
    {
        // Release references to resources related to the old device.
        dxgiDevice = nullptr;
        dxgiAdapter = nullptr;
        dxgiFactory = nullptr;
        previousDefaultAdapter = nullptr;

        // Create a new device and swap chain.
        HandleDeviceLost();
    }
}

/// <summary>
/// HandleDeviceLost
/// </summary>
void DX::DeviceResources::HandleDeviceLost()
{
    m_swapChain = nullptr;

    if (m_deviceNotify != nullptr)
    {
        m_deviceNotify->OnDeviceLost();
    }

    // Make sure the rendering state has been released.
    m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_d3dDepthStencilView = nullptr;
    m_d3dRenderTargetView = nullptr;
    m_d3dContext->Flush();

    CreateDevice();
    CreateWindowSizeDependentResources();

    if (m_deviceNotify != nullptr)
    {
        m_deviceNotify->OnDeviceRestored();
    }
}

/// <summary>
/// IDeviceNotify informs the DeviceResources class if the device has been lost or created.
/// </summary>
void DX::DeviceResources::RegisterDeviceNotify(DX::IDeviceNotify* deviceNotify)
{
    m_deviceNotify = deviceNotify;
}

/// <summary>
/// Invoked when the app suspends. It provides a hint to the driver that the app 
/// is entering an idle state and that temporary buffers can be reclaimed for 
/// use by other apps.
/// </summary>
void DX::DeviceResources::Trim()
{
    auto dxgiDevice{ m_d3dDevice.as<IDXGIDevice3>() };
    dxgiDevice->Trim();
}

#if defined(_DEBUG)
// Check for SDK Layer support.
// D3D11_CREATE_DEVICE_DEBUG creates a device that supports the debug layer.
// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-layers#debug-layer
inline bool SdkLayersAvailable()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL,       // no need to create a real device
        0,
        D3D11_CREATE_DEVICE_DEBUG,  // check for the SDK layers
        nullptr,                    // no need to check for feature level
        0,
        D3D11_SDK_VERSION,          // always D3D11_SDK_VERSION for WinRT apps
        nullptr,                    // no need to keep the D3D device reference
        nullptr,                    // no need to know the feature level
        nullptr                     // no need to keep the D3D device context reference
    );

    return SUCCEEDED(hr);
}
#endif
