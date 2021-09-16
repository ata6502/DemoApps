#pragma once

#include "SharedShaderStructures.h"

// Used to send per-vertex data to the vertex shader.
struct VertexPositionColor
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
};
