#include "pch.h"
#include "DemoMain.h"
#include "Utilities.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace DirectX;

DemoMain::DemoMain() :
    m_constantBufferData(),
    m_indexCount(0),
    m_initialized(false)
{
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
    
    m_input = std::make_unique<IndependentInput>();

    // Initialize device resources asynchronously.
    InitializeInBackground();

    m_timer.Reset();
}

DemoMain::~DemoMain()
{
    // Deregister device notification.
    m_deviceResources->RegisterDeviceNotify(nullptr);

    // Stop rendering and processing events on destruction.
    StopRenderLoop();
    m_input->StopProcessEvents();
}

winrt::fire_and_forget DemoMain::InitializeInBackground()
{
    auto lifetime = get_strong();

    // [1] Load shader bytecode.
    auto vertexShaderBytecode = co_await ReadDataAsync(L"VertexShader.cso");
    auto pixelShaderBytecode = co_await ReadDataAsync(L"PixelShader.cso");

    // [2] Create vertex shader.
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateVertexShader(
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            nullptr,
            m_vertexShader.put()));

    // [3] Create vertex description.
    static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // [4] Create the input layout using the vertex description and the vertex shader bytecode.
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateInputLayout(
            vertexDesc,
            ARRAYSIZE(vertexDesc),
            vertexShaderBytecode.data(),
            vertexShaderBytecode.Length(),
            m_inputLayout.put()));

    // [5] Create the pixel shader.
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreatePixelShader(
            pixelShaderBytecode.data(),
            pixelShaderBytecode.Length(),
            nullptr,
            m_pixelShader.put()));

    // [6] Create the constant buffer.
    CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            m_constantBuffer.put()));

    // [7] Create cube vertices. Each vertex has a position and a color.
    static const VertexPositionColor cubeVertices[] =
    {
        {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
        {XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
        {XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
        {XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
        {XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
        {XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
    };

    // [8] Create vertex buffer and load data.
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = cubeVertices;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            m_vertexBuffer.put()));

    // [9] Create cube indices in the left-handed coordinate system.
    static const unsigned short cubeIndices[] =
    {
        0,1,2, // -x
        1,3,2,

        4,6,5, // +x
        5,6,7,

        0,5,1, // -y
        0,4,5,

        2,7,6, // +y
        2,3,7,

        0,6,4, // -z
        0,2,6,

        1,7,3, // +z
        1,5,7,
    };

    // [10] Keep the number of indices.
    m_indexCount = ARRAYSIZE(cubeIndices);

    // [11] Create index buffer and load indices to the buffer.
    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = cubeIndices;
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
    winrt::check_hresult(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexBufferData,
            m_indexBuffer.put()));

    // Inform other parts of the application that the initialization has completed.
    m_initialized = true;
}

void DemoMain::CreateWindowSizeDependentResources()
{
    Size outputSize = m_deviceResources->GetOutputSize();

    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f);

    XMStoreFloat4x4(
        &m_constantBufferData.projection,
        XMMatrixTranspose(perspectiveMatrix));

    XMStoreFloat4x4(&m_constantBufferData.model,
        XMMatrixTranspose(XMMatrixIdentity()));
}

void DemoMain::StartRenderLoop()
{
    // Do not start another thread if the render loop is already running.
    if (m_renderLoopWorker != nullptr && m_renderLoopWorker.Status() == AsyncStatus::Started)
        return;

    m_timer.Start();

    // Create a task that will be run on a background thread.
    auto workItemHandler = ([this](IAsyncAction const& action)
    {
        // Calculate the updated frame and render once per vertical blanking interval.
        while (action.Status() == AsyncStatus::Started)
        {
            critical_section::scoped_lock lock(m_criticalSection);

            Update();
            Render();

            m_deviceResources->Present();

            m_timer.Update();
        }
    });

    // Run the task on a dedicated high priority background thread.
    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DemoMain::StopRenderLoop()
{
    m_timer.Stop();
    m_renderLoopWorker.Cancel();
}

void DemoMain::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel)
{
    m_deviceResources->SetSwapChainPanel(panel);
    m_input->Initialize(panel);
}

void DemoMain::SetLogicalSize(winrt::Windows::Foundation::Size const& logicalSize)
{
    m_deviceResources->SetLogicalSize(logicalSize);
}

void DemoMain::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
    m_deviceResources->SetCompositionScale(compositionScaleX, compositionScaleY);
}

void DemoMain::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}

void DemoMain::Suspend()
{
    StopRenderLoop();
    m_deviceResources->Trim();
}

void DemoMain::Resume()
{
    StartRenderLoop();
}

void DemoMain::OnDeviceLost()
{
    StopRenderLoop();
    ReleaseResources();
}

void DemoMain::OnDeviceRestored()
{
    InitializeInBackground();
    CreateWindowSizeDependentResources(); 
    StartRenderLoop();
}

void DemoMain::Update()
{
    XMVECTOR eye = m_input->GetPosition();
    static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
    static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMStoreFloat4x4(&m_constantBufferData.view,
        XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

/// <summary>
/// Renders the current frame.
/// </summary>
void DemoMain::Render()
{
    // Don't render anything before the first Update.
    if (fabs(m_timer.GetTotalSeconds()) < std::numeric_limits<float>::epsilon())
        return;

    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Reset render targets to the screen.
    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
    
    // Clear the back buffer and depth stencil view.
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);

    if (m_initialized)
    {
        // Prepare the constant buffer to send it to the graphics device.
        context->UpdateSubresource(m_constantBuffer.get(), 0, nullptr, &m_constantBufferData, 0, 0);

        // Each vertex is one instance of the VertexPositionColor struct.
        UINT stride = sizeof(VertexPositionColor);
        UINT offset = 0;
        ID3D11Buffer* pVertexBuffer{ m_vertexBuffer.get() };
        context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

        // Each index is one 16-bit unsigned integer (short).
        context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetInputLayout(m_inputLayout.get());

        // Attach our vertex shader.
        context->VSSetShader(m_vertexShader.get(), nullptr, 0);

        // Send the constant buffer to the graphics device.
        ID3D11Buffer* pConstantBuffer{ m_constantBuffer.get() };
        context->VSSetConstantBuffers(0, 1, &pConstantBuffer);

        // Attach our pixel shader.
        context->PSSetShader(m_pixelShader.get(), nullptr, 0);

        // Draw the cube.
        context->DrawIndexed(m_indexCount, 0, 0);
    }
}

void DemoMain::ReleaseResources()
{
    m_initialized = false;
    m_vertexShader = nullptr;
    m_inputLayout = nullptr;
    m_pixelShader = nullptr;
    m_constantBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}
