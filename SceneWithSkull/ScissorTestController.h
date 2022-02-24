#pragma once

#include "DeviceResources.h"

class ScissorTestController
{
public:
    ScissorTestController::ScissorTestController(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    bool IsScissorTestEnabled() const { return m_isScissorTestEnabled; }
    void SetOutputSize(winrt::Windows::Foundation::Size outputSize);
    void EnableScissorTest(bool enabled);
    void SetScissorTestLeftRightMargin(float marginPercent);
    void SetScissorTestTopBottomMargin(float marginPercent);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerStateScissorTestEnabled;
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerStateScissorTestDisabled;

    winrt::Windows::Foundation::Size        m_outputSize;
    float                                   m_leftRightMarginPercent;
    float                                   m_topBottomMarginPercent;
    bool                                    m_isScissorTestEnabled;

    void Initialize();
    void SetScissorTestRectangle();
};

