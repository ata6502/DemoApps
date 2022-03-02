#pragma once

#include "DeviceResources.h"

class ScissorTestController
{
public:
    ScissorTestController::ScissorTestController(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    void Initialize();
    void ReleaseResources();

    void StoreScissorTestState(bool enabled);
    void RefreshScissorTestState();
    void DisableScissorTest();
    void SetOutputSize(winrt::Windows::Foundation::Size outputSize);
    void SetScissorTestLeftRightMargin(float marginPercent);
    void SetScissorTestTopBottomMargin(float marginPercent);

private:
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerStateScissorTestEnabled;
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerStateScissorTestDisabled;

    bool                                    m_isScissorTestEnabled;
    winrt::Windows::Foundation::Size        m_outputSize;
    float                                   m_leftRightMarginPercent;
    float                                   m_topBottomMarginPercent;

    void SetScissorTestRectangle();
};

