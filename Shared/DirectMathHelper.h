#pragma once

// Computes a face normal of a triangle P1P2P3.
inline DirectX::XMVECTOR ComputeNormal(const DirectX::FXMVECTOR& p0, const DirectX::FXMVECTOR& p1, const DirectX::FXMVECTOR& p2)
{
    using namespace DirectX;

    XMVECTOR u = p1 - p0;
    XMVECTOR v = p2 - p0;
    return XMVector3Normalize(XMVector3Cross(u, v));
}

// Converts XMVECTOR to XMFLOAT3
inline DirectX::XMFLOAT3 VectorToFloat3(const DirectX::FXMVECTOR& v)
{
    using namespace DirectX;

    XMFLOAT3 f;
    XMStoreFloat3(&f, v);
    return f;
}

// Computes the inverse transpose matrix.
static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
{
    using namespace DirectX;

    // Zero out the translation row in the input matrix because 
    // we apply an inverse-transpose matrix just to normals. 
    // We don't want the inverse-transpose of the translation.
    XMMATRIX A = M;
    A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMVECTOR det = XMMatrixDeterminant(A);
    return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

#if defined(_DEBUG)
// Examples: 
// DebugTrace(L"num = %4.2f\n", num);
// DebugTrace(L"%s\n", str.c_str());
inline void DebugTrace(const wchar_t* format, ...)
{
    // Generate the message string.
    va_list args;
    va_start(args, format); // initialize the argument list
    wchar_t buffer[1024];
    ASSERT(_vsnwprintf_s(buffer, _countof(buffer) - 1, format, args) != -1);
    va_end(args);

    OutputDebugStringW(buffer); // this is a Windows function
}

inline void OutputDebug(DirectX::CXMMATRIX matrix)
{
    DirectX::XMFLOAT4X4 m;
    XMStoreFloat4x4(&m, matrix);
    DebugTrace(L"%2.2f %2.2f %2.2f %2.2f\n", m._11, m._12, m._13, m._14);
    DebugTrace(L"%2.2f %2.2f %2.2f %2.2f\n", m._21, m._22, m._23, m._24);
    DebugTrace(L"%2.2f %2.2f %2.2f %2.2f\n", m._31, m._32, m._33, m._34);
    DebugTrace(L"%2.2f %2.2f %2.2f %2.2f\n", m._41, m._42, m._43, m._44);
    DebugTrace(L"\n");
}

inline void OutputDebug(DirectX::FXMVECTOR vector)
{
    DirectX::XMFLOAT4 v;
    XMStoreFloat4(&v, vector);
    DebugTrace(L"%2.2f %2.2f %2.2f %2.2f\n", v.x, v.y, v.z, v.w);
    DebugTrace(L"\n");
}
#endif
