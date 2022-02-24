#include "pch.h"
#include "ScissorTestController.h"

ScissorTestController::ScissorTestController(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources),
    m_outputSize()
{ 
    Initialize();
}

void ScissorTestController::Initialize()
{
    auto device{ m_deviceResources->GetD3DDevice() };

    // Create rasterizer states to enable or diable the scissor test using 
    // the D3D11_RASTERIZER_DESC::ScissorEnable flag.
    D3D11_RASTERIZER_DESC2 rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC2));
    rsDesc.AntialiasedLineEnable = false;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.DepthClipEnable = true;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.MultisampleEnable = false;
    rsDesc.ScissorEnable = true; // enable the scissor test
    rsDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state to enable the scissor test.
    winrt::check_hresult(
        device->CreateRasterizerState2(&rsDesc, m_rasterizerStateScissorTestEnabled.put()));

    // Create the rasterizer state to disable the scissor test.
    rsDesc.ScissorEnable = false; // disable the scissor test
    winrt::check_hresult(
        device->CreateRasterizerState2(&rsDesc, m_rasterizerStateScissorTestDisabled.put()));
}

void ScissorTestController::SetOutputSize(winrt::Windows::Foundation::Size outputSize)
{
    m_outputSize = outputSize;
}

/// <summary>
/// Sends an array of screen rectangles (in this example, there is only one rectangle in the array)
/// to the Direct3D scissor test. The scissor test discards all pixels outside the scissor rectangles.
/// </summary>
void ScissorTestController::EnableScissorTest(bool enabled)
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    // Set the rasterizer state.
    if (enabled)
        context->RSSetState(m_rasterizerStateScissorTestEnabled.get());
    else
        context->RSSetState(m_rasterizerStateScissorTestDisabled.get());
}

void ScissorTestController::SetScissorTestLeftRightMargin(float marginPercent)
{
    m_leftRightMarginPercent = marginPercent;
    SetScissorTestRectangle();
}

void ScissorTestController::SetScissorTestTopBottomMargin(float marginPercent)
{
    m_topBottomMarginPercent = marginPercent;
    SetScissorTestRectangle();
}

void ScissorTestController::SetScissorTestRectangle()
{
    auto context{ m_deviceResources->GetD3DDeviceContext() };

    auto leftRightMargin = static_cast<long>((m_leftRightMarginPercent / 100.0f) * m_outputSize.Width);
    auto topBottomMargin = static_cast<long>((m_topBottomMarginPercent / 100.0f) * m_outputSize.Width);

    // Set the scissor test rectangle.
    D3D11_RECT rects = {
        leftRightMargin,
        topBottomMargin,
        static_cast<long>(m_outputSize.Width) - leftRightMargin,
        static_cast<long>(m_outputSize.Height) - topBottomMargin };

    context->RSSetScissorRects(1, &rects);
}
