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
    virtual void Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float elapsedSeconds) = 0;
    virtual float GetDistanceToCamera() = 0;
    virtual float GetCameraPitch() = 0;

protected:
    void IsInitialized(bool isInitialized) { m_initialized = isInitialized; }

private:
    bool m_initialized;
};