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
    void SetViewMatrix(DirectX::FXMMATRIX viewMatrix, DirectX::FXMVECTOR eyePosition, float totalSeconds);
    void SetOutputSize(winrt::Windows::Foundation::Size outputSize);

    void EnableScissorTest(bool enabled);
    void SetScissorTestLeftRightMargin(float marginPercent);
    void SetScissorTestTopBottomMargin(float marginPercent);

private:
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
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerStateScissorTestEnabled;
    winrt::com_ptr<ID3D11RasterizerState2>  m_rasterizerStateScissorTestDisabled;

    // Constant buffers.
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferNeverChanges;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerFrame;
    winrt::com_ptr<ID3D11Buffer>            m_constantBufferPerObject;

    winrt::Windows::Foundation::Size        m_outputSize;
    DirectX::XMFLOAT4X4                     m_projMatrix;
    std::unique_ptr<MeshGenerator>          m_meshGenerator;
    std::vector<ObjectInfo>                 m_objects;
    float                                   m_leftRightMarginPercent;
    float                                   m_topBottomMarginPercent;

    void SetObjectData(DirectX::FXMMATRIX worldMatrix);
    void CreateMeshes();
    void DefineSceneObjects();
    void SetScissorTestRectangle();
};

