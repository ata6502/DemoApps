#include "pch.h"
#include "FogController.h"

const float FogController::DEFAULT_FOG_START = 20.0f;
const float FogController::DEFAULT_FOG_RANGE = 170.0f;

FogController::FogController() :
    m_fogStart(DEFAULT_FOG_START),
    m_fogRange(DEFAULT_FOG_RANGE)
{
    m_fogColor = DirectX::XMFLOAT4(.4f, .4f, .4f, 1.0f);
}

void FogController::SetFogStart(float fogStart)
{
    m_fogStart = fogStart;
}

void FogController::SetFogRange(float fogRange)
{
    m_fogRange = fogRange;
}
