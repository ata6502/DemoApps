#pragma once
#include <map>

#include "DeviceResources.h"
#include "LightsShaderStructures.h"
#include "MeshGeneratorTexture.h"
#include "RendererBase.h"

class TextureRenderer : public RendererBase
{
public:
    TextureRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources);

    winrt::Windows::Foundation::IAsyncAction InitializeInBackground();
    void Render();
    void ReleaseResources();

    void FinalizeInitialization();
    void SetProjMatrix(DirectX::FXMMATRIX projMatrix);
    void Update(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float elapsedSeconds);
    float GetDistanceToCamera();
    float GetCameraPitch();
    bool IsScissorTestSupported() { return false; }

private:
    static const float DISTANCE_TO_CAMERA;
    static const float CAMERA_PITCH;

    struct ObjectInfo
    {
        std::string MeshName;
        MaterialDesc Material;
        DirectX::XMFLOAT4X4 WorldMatrix;
    };

    std::shared_ptr<DX::DeviceResources>   m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11InputLayout>      m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>     m_vertexShader;
    winrt::com_ptr<ID3D11PixelShader>      m_pixelShader;
    winrt::com_ptr<ID3D11RasterizerState2> m_rasterizerStateScissorTestEnabled;
    winrt::com_ptr<ID3D11RasterizerState2> m_rasterizerStateScissorTestDisabled;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>           m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>           m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>           m_constantBufferPerObject;

    DirectX::XMFLOAT4X4                    m_projMatrix;
    std::unique_ptr<MeshGeneratorTexture>  m_meshGenerator;
    std::map<std::string, ObjectInfo>      m_objects;
    float                                  m_rotation;
    float                                  m_leftRightMarginPercent;
    float                                  m_topBottomMarginPercent;

    void SetObjectData(std::string const& name, ObjectInfo const& info);
    winrt::Windows::Foundation::IAsyncAction CreateMeshes();
    void DefineSceneObjects();
};

