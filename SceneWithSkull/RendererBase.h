#pragma once

class RendererBase
{
public:
    RendererBase() : m_initialized(false) {}
    virtual ~RendererBase() {}

    virtual winrt::Windows::Foundation::IAsyncAction InitializeInBackground() = 0;
    virtual void Render() = 0;
    virtual void ReleaseResources() = 0;

    bool IsInitialized() const { return m_initialized; }

    virtual void FinalizeInitialization() = 0;
    virtual void SetProjMatrix(DirectX::FXMMATRIX projMatrix) = 0;
    virtual void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds) = 0;
    virtual void SetOutputSize(winrt::Windows::Foundation::Size outputSize) = 0;

    virtual void EnableScissorTest(bool enabled) = 0;
    virtual void SetScissorTestLeftRightMargin(float marginPercent) = 0;
    virtual void SetScissorTestTopBottomMargin(float marginPercent) = 0;

protected:
    void IsInitialized(bool isInitialized) { m_initialized = isInitialized; }

private:
    bool m_initialized;
};