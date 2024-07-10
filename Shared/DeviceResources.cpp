#include "pch.h"

#include "DeviceResources.h"

#include <windows.ui.xaml.media.dxinterop.h>

using namespace D2D1;
using namespace DirectX;
using namespace DX;
using namespace winrt::Windows::Graphics::Display; // DisplayOrientations

#if defined(_DEBUG)
inline bool SdkLayersAvailable();
#endif

// Constants used to calculate screen rotations.
namespace ScreenRotation
{
    // 0-degree Z-rotation
    static const XMFLOAT4X4 Rotation0(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 90-degree Z-rotation
    static const XMFLOAT4X4 Rotation90(
        0.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 180-degree Z-rotation
    static const XMFLOAT4X4 Rotation180(
        -1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 270-degree Z-rotation
    static const XMFLOAT4X4 Rotation270(
        0.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
};

DeviceResources::DeviceResources(
    DXGI_FORMAT backBufferFormat,
    DXGI_FORMAT depthBufferFormat) :
    m_screenViewport{},
    m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1), // 32-bit index buffer requires D3D_FEATURE_LEVEL_9_2
    m_outputSize{},
    m_logicalSize{},
    m_d3dRenderTargetSize{},
    m_nativeOrientation(DisplayOrientations::None),
    m_currentOrientation(DisplayOrientations::None),
    m_compositionScaleX(1.0f),
    m_compositionScaleY(1.0f),
    m_dpi(-1.0f),
    m_deviceNotify(nullptr),
    m_backBufferFormat(backBufferFormat),
    m_depthBufferFormat(depthBufferFormat)
{
    CreateDeviceIndependentResources();
    CreateDeviceResources();
}

/// <summary>
/// Configures resources that don't depend on the Direct3D device.
/// </summary>
void DeviceResources::CreateDeviceIndependentResources()
{
    // Initialize Direct2D resources.
    D2D1_FACTORY_OPTIONS options;
    ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
    // If the project is in a debug build, enable Direct2D debugging via SDK Layers.
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    // Initialize the Direct2D Factory.
    // For info on put_void see: 
    // https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/consume-com#com-functions-that-return-an-interface-pointer-as-void
    // https://learn.microsoft.com/en-us/uwp/cpp-ref-for-winrt/com-ptr#com_ptrput_void-function
    winrt::check_hresult(
        D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(m_d2dFactory), // __uuidof(ID2D1Factory3)
            &options,
            m_d2dFactory.put_void() // void** type
        )
    );

    // Initialize the DirectWrite Factory.
    winrt::check_hresult(
        DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_dwriteFactory), // __uuidof(IDWriteFactory3)
            reinterpret_cast<IUnknown**>(m_dwriteFactory.put()) // (::IUnknown**)m_dwriteFactory.put_void()
        )
    );

    // Initialize the Windows Imaging Component (WIC) cactory (CoCreateInstance).
    m_wicFactory = winrt::create_instance<IWICImagingFactory2>(CLSID_WICImagingFactory2);
}

/// <summary>
/// Creates Direct3D device and device context. 
/// </summary>
void DeviceResources::CreateDeviceResources()
{
    // This flag allows to create a device that can interoperate with Direct2D API. 
    // It adds support for surfaces with a different color channel ordering than the API default.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    if (SdkLayersAvailable())
    {
        // If the project is in a debug build, enable debugging via SDK Layers.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
    else
    {
        OutputDebugStringA("WARNING: Direct3D Debug Device is not available.\n");
    }
#endif

    // Define an array of feature levels this app supports. The array elements are arranged in the order
    // of preference. Each feature level is a superset of the previous feature level i.e., a higher-level 
    // CPU supports the lower feature levels.
    // All applications are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2, // any use of 32-bit index buffers (DXGI_FORMAT_R32_UINT) requires at least D3D_FEATURE_LEVEL_9_2
        D3D_FEATURE_LEVEL_9_1
    };

    // Create the Direct3D device and the device immediate context.
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // use the hardware graphics driver
        0,                          // a handle to the software driver DLL; always 0 except for D3D_DRIVER_TYPE_SOFTWARE
        creationFlags,              // special features enabled when the device is created
        featureLevels,              // a list of feature levels this app can support
        ARRAYSIZE(featureLevels),   // the size of the feature level list
        D3D11_SDK_VERSION,          // always D3D11_SDK_VERSION for WinRT apps
        device.put(),               // returns the Direct3D device created
        &m_d3dFeatureLevel,         // returns the feature level of device created
        context.put());             // returns the device immediate context

    if (FAILED(hr))
    {
        // If the initialization failed, fall back to the WARP device. The WARP device is a software
        // renderer optimized for rendering speed. It uses special features of a CPU, such as multiple
        // cores and SIMD instructions. 
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
                &m_d3dFeatureLevel,
                context.put()
            )
        );
    }

    // Store pointers to the Direct3D 11.3 API device and the immediate context.
    m_d3dDevice = device.as<ID3D11Device3>();
    m_d3dContext = context.as<ID3D11DeviceContext3>();

    // Create the Direct2D device object and a corresponding context.
    winrt::com_ptr<IDXGIDevice3> dxgiDevice = m_d3dDevice.as<IDXGIDevice3>();

    winrt::check_hresult(
        m_d2dFactory->CreateDevice(dxgiDevice.get(), m_d2dDevice.put())
    );

    winrt::check_hresult(
        m_d2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            m_d2dContext.put()
        )
    );
}

/// <summary>
/// Creates resources that depend on the window size. The resources need to be
/// recreated every time the window size changes.
/// </summary>
void DeviceResources::CreateWindowSizeDependentResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    m_d3dRenderTargetView = nullptr;
    m_d3dDepthStencilView = nullptr;
    m_renderTarget = nullptr;
    m_depthStencil = nullptr;
    m_d2dContext->SetTarget(nullptr);
    m_d2dTargetBitmap = nullptr;
    m_d3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

    // XAML apps.
    m_outputSize.Width = m_logicalSize.Width * m_compositionScaleX;
    m_outputSize.Height = m_logicalSize.Height * m_compositionScaleY;

    // Prevent zero size DirectX content from being created.
    m_outputSize.Width = std::max(m_outputSize.Width, 1.f);
    m_outputSize.Height = std::max(m_outputSize.Height, 1.f);

    // The width and height of the swap chain must be based on the window's
    // natively-oriented width and height. If the window is not in the native
    // orientation, the dimensions must be reversed.
    // We do this to take advantage on an optimization that allows for the 
    // application to draw the content in the proper orientation and then
    // indicate to DXGI that the content has been pre-rotated.
    DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

    bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
    m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
    m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

    if (m_swapChain != nullptr)
    {
        // If the swap chain already exists, resize it.
        HRESULT hr = m_swapChain->ResizeBuffers(
            2, // double-buffered swap chain
            lround(m_d3dRenderTargetSize.Width),
            lround(m_d3dRenderTargetSize.Height),
            m_backBufferFormat,
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
    else
    {
        // Create a swap chain using the same adapter as the Direct3D device.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

        swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width);          // match the size of the window
        swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
        swapChainDesc.Format = m_backBufferFormat;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;                                 // don't use multi-sampling
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;                                      // use double-buffering
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;        // Windows Store apps must use FLIP SwapEffects
        swapChainDesc.Flags = 0;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;                       // when using XAML interop, use DXGI_SCALING_STRETCH scaling

        // Get the DXGI device.
        winrt::com_ptr<IDXGIDevice3> dxgiDevice = m_d3dDevice.as<IDXGIDevice3>();

        // Get the DXGI adapter.
        winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
        winrt::check_hresult(
            dxgiDevice->GetAdapter(dxgiAdapter.put())
        );

        // Get the DXGI factory.
        winrt::com_ptr<IDXGIFactory3> dxgiFactory;
        winrt::check_hresult(
            dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
        );

        // Alternative code using winrt::capture
        //winrt::com_ptr<IDXGIFactory3> dxgiFactory =
        //    winrt::capture<IDXGIFactory3>(dxgiAdapter, &IDXGIAdapter::GetParent);

        // When using XAML interop, the swap chain must be created for composition.
        winrt::check_hresult(
            dxgiFactory->CreateSwapChainForComposition(
                m_d3dDevice.get(),
                &swapChainDesc,
                nullptr,
                m_swapChain.put()
            )
        );

        // Associate swap chain with SwapChainPanel. UI changes need to be dispatched back to the UI thread.
        m_swapChainPanel.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::High, [=]()
        {
            // Original code:
            //     ComPtr<ISwapChainPanelNative> panelNative;
            //     DX::ThrowIfFailed(reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative)));

            // Get an ISwapChainPanelNative from the SwapChainPanel UI control.
            auto panelNative{ m_swapChainPanel.as<ISwapChainPanelNative>() };

            // Associate the swap chain with the SwapChainPanel UI control.
            winrt::check_hresult(
                panelNative->SetSwapChain(m_swapChain.get())
            );
        });

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
        // ensures that the application will only render after each VSync, minimizing power consumption.
        winrt::check_hresult(
            dxgiDevice->SetMaximumFrameLatency(1));
    }

    // Set the proper orientation for the swap chain, and generate 2D and
    // 3D matrix transformations for rendering to the rotated swap chain.
    // Note the rotation angle for the 2D and 3D transforms are different.
    // This is due to the difference in coordinate spaces.  Additionally,
    // the 3D matrix is specified explicitly to avoid rounding errors.
    switch (displayRotation)
    {
    case DXGI_MODE_ROTATION_IDENTITY:
        m_orientationTransform2D = Matrix3x2F::Identity();
        m_orientationTransform3D = ScreenRotation::Rotation0;
        break;

    case DXGI_MODE_ROTATION_ROTATE90:
        m_orientationTransform2D =
            Matrix3x2F::Rotation(90.0f) *
            Matrix3x2F::Translation(m_logicalSize.Height, 0.0f);
        m_orientationTransform3D = ScreenRotation::Rotation270;
        break;

    case DXGI_MODE_ROTATION_ROTATE180:
        m_orientationTransform2D =
            Matrix3x2F::Rotation(180.0f) *
            Matrix3x2F::Translation(m_logicalSize.Width, m_logicalSize.Height);
        m_orientationTransform3D = ScreenRotation::Rotation180;
        break;

    case DXGI_MODE_ROTATION_ROTATE270:
        m_orientationTransform2D =
            Matrix3x2F::Rotation(270.0f) *
            Matrix3x2F::Translation(0.0f, m_logicalSize.Width);
        m_orientationTransform3D = ScreenRotation::Rotation90;
        break;

    default:
        winrt::throw_hresult(E_FAIL);
    }

    winrt::check_hresult(
        m_swapChain->SetRotation(displayRotation)
    );

    // Setup inverse scale on the swap chain. Applies only to XAML apps.
    DXGI_MATRIX_3X2_F inverseScale = { 0 };
    inverseScale._11 = 1.0f / m_compositionScaleX;
    inverseScale._22 = 1.0f / m_compositionScaleY;
    auto swapChain{ m_swapChain.as<IDXGISwapChain2>() }; // cast to IDXGISwapChain2 to call SetMatrixTransform
    winrt::check_hresult(
        swapChain->SetMatrixTransform(&inverseScale));

    //
    // Create a render target view of the swap chain back buffer for use in the rendering pipeline.
    //

    // Acquire the texture interface from a swap chain.
    // ID3D11Texture2D* renderTarget = NULL;
    // HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&renderTarget);
    // -or-
    // winrt::com_ptr<ID3D11Texture2D> renderTarget;
    // winrt::check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget)));
    // -or-
    // ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(renderTarget.ReleaseAndGetAddressOf()))); ???
    m_renderTarget = winrt::capture<ID3D11Texture2D>(m_swapChain, &IDXGISwapChain1::GetBuffer, 0);

    // In order to use the texture interface acquired from the swap chain, we need a resource view to bind
    // the texture as a render target for the pipeline. The type of resource view we need is a render target view.

    winrt::check_hresult(
        m_d3dDevice->CreateRenderTargetView(
            m_renderTarget.get(),
            nullptr, // use a default view description; you can create a descrption using: CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, m_backBufferFormat);
            m_d3dRenderTargetView.put()));

    // Create a depth stencil buffer and a corresponding depth stencil view.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        m_depthBufferFormat,
        static_cast<UINT>(m_d3dRenderTargetSize.Width),
        static_cast<UINT>(m_d3dRenderTargetSize.Height),
        1, // use one texture for this depth-stencil view
        1, // use a single mipmap level
        D3D11_BIND_DEPTH_STENCIL);

    winrt::check_hresult(
        m_d3dDevice->CreateTexture2D(
            &depthStencilDesc,
            nullptr,
            m_depthStencil.put()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    winrt::check_hresult(
        m_d3dDevice->CreateDepthStencilView(
            m_depthStencil.get(),
            &depthStencilViewDesc,
            m_d3dDepthStencilView.put()));

    // Set the 3D rendering viewport to target the entire window.
    m_screenViewport = CD3D11_VIEWPORT(
        0.0f,
        0.0f,
        m_outputSize.Width,
        m_outputSize.Height);

    m_d3dContext->RSSetViewports(1, &m_screenViewport);

    // Create a Direct2D target bitmap associated with the
    // swap chain back buffer and set it as the current target.
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(m_backBufferFormat, D2D1_ALPHA_MODE_PREMULTIPLIED),
            m_dpi,
            m_dpi
        );

    winrt::com_ptr<IDXGIResource1> dxgiBackBuffer =
        winrt::capture<IDXGIResource1>(m_swapChain, &IDXGISwapChain1::GetBuffer, 0);

    // Alternative code that checks HRESULT:
    //winrt::com_ptr<IDXGIResource1> dxgiBackBuffer;
    //winrt::check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

    winrt::com_ptr<IDXGISurface2> dxgiSurface;
    winrt::check_hresult(
        dxgiBackBuffer->CreateSubresourceSurface(0, dxgiSurface.put())
    );

    winrt::check_hresult(
        m_d2dContext->CreateBitmapFromDxgiSurface(
            dxgiSurface.get(),
            &bitmapProperties,
            m_d2dTargetBitmap.put()
        )
    );

    m_d2dContext->SetTarget(m_d2dTargetBitmap.get());

    // Grayscale text anti-aliasing is recommended for all Windows Store apps.
    m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

/// <summary>
/// Caches a reference to a SwapChainPanel UI control as well as some properties related to 
/// the size of the panel. Called when the panel is created or re-created.
/// This method is used only in XAML apps.
/// </summary>
void DeviceResources::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel)
{
    m_swapChainPanel = panel;

    m_logicalSize = winrt::Windows::Foundation::Size(static_cast<float>(panel.ActualWidth()), static_cast<float>(panel.ActualHeight()));
    m_compositionScaleX = panel.CompositionScaleX();
    m_compositionScaleY = panel.CompositionScaleY();

    DisplayInformation displayInformation = DisplayInformation::GetForCurrentView();
    m_nativeOrientation = displayInformation.NativeOrientation();
    m_currentOrientation = displayInformation.CurrentOrientation();

    m_dpi = displayInformation.LogicalDpi();
    m_d2dContext->SetDpi(m_dpi, m_dpi);

    CreateWindowSizeDependentResources();
}

/// <summary>
/// Called in the event handler for the SwapChainPanel::SizeChanged event (in XAML apps)
/// or in CoreWindow::SizeChanged event (in Core Window apps).
/// </summary>
void DeviceResources::SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize)
{
    if (m_logicalSize != logicalSize)
    {
        m_logicalSize = logicalSize;
        CreateWindowSizeDependentResources();
    }
}

/// <summary>
/// Called in the event handler for the CompositionScaleChanged event. 
/// This method is used only in XAML apps.
/// </summary>
void DeviceResources::SetCompositionScale(float compositionScaleX, float compositionScaleY)
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
/// Called in the event handler for the OrientationChanged event.
/// </summary>
void DeviceResources::SetCurrentOrientation(winrt::Windows::Graphics::Display::DisplayOrientations currentOrientation)
{
    if (m_currentOrientation != currentOrientation)
    {
        m_currentOrientation = currentOrientation;
        CreateWindowSizeDependentResources();
    }
}

/// <summary>
/// Called in the event handler for the DpiChanged event.
/// </summary>
void DeviceResources::SetDpi(float dpi)
{
    if (dpi != m_dpi)
    {
        m_dpi = dpi;

        m_d2dContext->SetDpi(m_dpi, m_dpi);
        CreateWindowSizeDependentResources();
    }
}

/// <summary>
/// Presents the contents of the swap chain to the screen. 
/// </summary>
void DeviceResources::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    HRESULT hr = m_swapChain->Present1(1, 0, &parameters); // we could call Present(1, 0)

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be modified.
    m_d3dContext->DiscardView1(m_d3dRenderTargetView.get(), nullptr, 0); // we could call DiscardView(m_d3dRenderTargetView.get())

    // Discard the contents of the depth stencil.
    m_d3dContext->DiscardView1(m_d3dDepthStencilView.get(), nullptr, 0); // we could call DiscardView(m_d3dDepthStencilView.get())

    // If the device was removed either by a disconnection or a driver upgrade, 
    // we must recreate all device resources.
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
/// Called in the event handler for the DisplayContentsInvalidated event.
/// </summary>
void DeviceResources::ValidateDevice()
{
    // The D3D Device is no longer valid if the default adapter changed since the device
    // was created or if the device has been removed.

    // First, get the information for the default adapter from when the device was created.

    auto dxgiDevice{ m_d3dDevice.as<IDXGIDevice3>() };

    winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
    winrt::check_hresult(dxgiDevice->GetAdapter(dxgiAdapter.put()));

    winrt::com_ptr<IDXGIFactory3> dxgiFactory;
    winrt::check_hresult(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))); // equivalent code but without checking HRESULT: dxgiFactory.capture(dxgiAdapter, &IDXGIAdapter::GetParent);

    winrt::com_ptr<IDXGIAdapter1> previousDefaultAdapter;
    winrt::check_hresult(dxgiFactory->EnumAdapters1(0, previousDefaultAdapter.put()));

    DXGI_ADAPTER_DESC1 previousDesc;
    ZeroMemory(&previousDesc, sizeof(previousDesc));
    winrt::check_hresult(previousDefaultAdapter->GetDesc1(&previousDesc));

    // Next, get the information for the current default adapter.

    winrt::com_ptr<IDXGIFactory3> currentFactory;
    winrt::check_hresult(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory))); // equivalent code but without checking HRESULT: currentFactory.capture(&CreateDXGIFactory1);

    winrt::com_ptr<IDXGIAdapter1> currentDefaultAdapter;
    winrt::check_hresult(currentFactory->EnumAdapters1(0, currentDefaultAdapter.put()));

    DXGI_ADAPTER_DESC currentDesc;
    ZeroMemory(&currentDesc, sizeof(currentDesc));
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
/// Invoked when the app suspends. It provides a hint to the driver that the app is entering 
/// an idle state and that temporary buffers can be reclaimed for use by other apps.
/// </summary>
void DeviceResources::Trim()
{
    auto dxgiDevice{ m_d3dDevice.as<IDXGIDevice3>() };
    dxgiDevice->Trim();
}

/// <summary>
/// IDeviceNotify informs the DeviceResources class if the device has been lost or created.
/// </summary>
void DeviceResources::RegisterDeviceNotify(IDeviceNotify* deviceNotify)
{
    m_deviceNotify = deviceNotify;
}

/// <summary>
/// Recreate all device resources and set them back to the current state.
/// </summary>
void DeviceResources::HandleDeviceLost()
{
    m_swapChain = nullptr;

    if (m_deviceNotify != nullptr)
    {
        // Notify the renderers that device resources need to be released.
        // This ensures all references to the existing swap chain are released so that a new one can be created.
        m_deviceNotify->OnDeviceLost();
    }

    // Make sure the rendering state has been released.
    m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_d3dDepthStencilView = nullptr;
    m_d3dRenderTargetView = nullptr;
    m_renderTarget = nullptr;
    m_depthStencil = nullptr;

    m_d2dContext->SetTarget(nullptr);
    m_d2dTargetBitmap = nullptr;
    m_d2dContext = nullptr;
    m_d2dDevice = nullptr;

    m_d3dContext->Flush();

    // Create the new device and swap chain.
    CreateDeviceResources();
    m_d2dContext->SetDpi(m_dpi, m_dpi);
    CreateWindowSizeDependentResources();

    if (m_deviceNotify != nullptr)
    {
        // Notify the renderers that resources can now be created again.
        m_deviceNotify->OnDeviceRestored();
    }
}

/// <summary>
/// Determines the rotation between the display device's native Orientation and the current display orientation.
/// </summary>
DXGI_MODE_ROTATION DeviceResources::ComputeDisplayRotation()
{
    DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

    // Note: NativeOrientation can only be Landscape or Portrait even though
    // the DisplayOrientations enum has other values.
    switch (m_nativeOrientation)
    {
    case DisplayOrientations::Landscape:
        switch (m_currentOrientation)
        {
        case DisplayOrientations::Landscape:
            rotation = DXGI_MODE_ROTATION_IDENTITY;
            break;

        case DisplayOrientations::Portrait:
            rotation = DXGI_MODE_ROTATION_ROTATE270;
            break;

        case DisplayOrientations::LandscapeFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE180;
            break;

        case DisplayOrientations::PortraitFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE90;
            break;
        }
        break;

    case DisplayOrientations::Portrait:
        switch (m_currentOrientation)
        {
        case DisplayOrientations::Landscape:
            rotation = DXGI_MODE_ROTATION_ROTATE90;
            break;

        case DisplayOrientations::Portrait:
            rotation = DXGI_MODE_ROTATION_IDENTITY;
            break;

        case DisplayOrientations::LandscapeFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE270;
            break;

        case DisplayOrientations::PortraitFlipped:
            rotation = DXGI_MODE_ROTATION_ROTATE180;
            break;
        }
        break;
    }
    return rotation;
}

#if defined(_DEBUG)
// Check for SDK Layer support.
// D3D11_CREATE_DEVICE_DEBUG creates a device that supports the debug layer.
// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-layers#debug-layer
inline bool SdkLayersAvailable()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL,       // no need to create a real hardware device
        0,
        D3D11_CREATE_DEVICE_DEBUG,  // check for the SDK layers
        nullptr,                    // no need to check for feature level
        0,
        D3D11_SDK_VERSION,          // always D3D11_SDK_VERSION for UWP apps
        nullptr,                    // no need to keep the D3D device reference
        nullptr,                    // no need to know the feature level
        nullptr                     // no need to keep the D3D device context reference
    );

    return SUCCEEDED(hr);
}
#endif
