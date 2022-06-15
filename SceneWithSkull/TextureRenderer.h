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
    bool IsThreeLightSystemSupported() { return true; }

private:
    static const float DISTANCE_TO_CAMERA;
    static const float CAMERA_PITCH;

    struct ObjectInfo
    {
        std::string MeshName;
        std::string TextureName;
        MaterialDesc Material;
        DirectX::XMFLOAT4X4 WorldMatrix;
    };

    std::shared_ptr<DX::DeviceResources>   m_deviceResources;

    // Direct3D resources.
    winrt::com_ptr<ID3D11InputLayout>      m_inputLayout;
    winrt::com_ptr<ID3D11VertexShader>     m_vertexShader;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>           m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>           m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>           m_constantBufferPerObject;

    // [Luna] Ex.7 p.310 Modify the "Lit Skull" demo by adding textures to the ground, columns, and spheres.

    // Direct3D objects used with textures.
    std::map<std::string, winrt::com_ptr<ID3D11ShaderResourceView>> m_textures;
    winrt::com_ptr<ID3D11SamplerState>     m_linearSampler;

    DirectX::XMFLOAT4X4                    m_projMatrix;
    std::unique_ptr<MeshGeneratorTexture>  m_meshGenerator;
    std::map<std::string, ObjectInfo>      m_objects;
    float                                  m_rotation;
    DirectX::XMFLOAT4X4                    m_textureTransform;

    void SetObjectData(std::string const& name, ObjectInfo const& info);
    winrt::Windows::Foundation::IAsyncAction CreateMeshes();
    void DefineSceneObjects();
};

