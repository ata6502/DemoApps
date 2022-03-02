#pragma once

#include "DeviceResources.h"
#include "MeshGenerator.h"
#include "RendererBase.h"

class ColorRenderer : public RendererBase
{
public:
    ColorRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float elapsedSeconds);
    float GetDistanceToCamera();
    float GetCameraPitch();
    bool IsScissorTestSupported() { return true; }
    bool IsThreeLightSystemSupported() { return false; }

private:
    static const float DISTANCE_TO_CAMERA;
    static const float CAMERA_PITCH;

    struct ObjectInfo
    {
        std::string MeshName;
        DirectX::XMFLOAT4X4 WorldMatrix;
    };

    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11InputLayout>       m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>      m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>       m_pixelShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;

    DirectX::XMFLOAT4X4                     m_projMatrix;
    std::unique_ptr<MeshGenerator>          m_meshGenerator;
    std::vector<ObjectInfo>                 m_objects;

    void SetObjectData(DirectX::FXMMATRIX worldMatrix);
    void CreateMeshes();
    void DefineSceneObjects();
};

