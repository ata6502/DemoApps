#pragma once

interface IRenderer
{
    virtual winrt::Windows::Foundation::IAsyncAction InitializeInBackground() = 0;
    virtual void Render() = 0;
    virtual void ReleaseResources() = 0;

    virtual void FinalizeInitialization() = 0;
    virtual void SetProjMatrix(DirectX::FXMMATRIX projMatrix) = 0;
    virtual void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds) = 0;
    virtual void SetWorldMatrix(DirectX::FXMMATRIX worldMatrix) = 0;
};