#pragma once

/// <summary>
/// This code is from the [Luna] book by Frank Luna (C) 2011 All Rights Reserved.
/// 
/// Performs the calculations for the wave simulation. After the simulation has been
/// updated, the client must copy the current solution into vertex buffers for rendering.
/// This class only does the calculations, it does not do any drawing.
/// </summary>
class Waves
{
public:
    Waves();
    ~Waves();

    uint32_t RowCount() const;
    uint32_t ColumnCount() const;
    uint32_t VertexCount() const;
    uint32_t TriangleCount() const;
    float Width() const;
    float Depth() const;

    // Returns the solution at the i-th grid point.
    const DirectX::XMFLOAT3& operator[](int i) const { return mCurrSolution[i]; }

    // Returns the solution normal at the ith grid point.
    const DirectX::XMFLOAT3& Normal(int i) const { return mNormals[i]; }

    // Returns the unit tangent vector at the ith grid point in the local x-axis direction.
    const DirectX::XMFLOAT3& TangentX(int i) const { return mTangentX[i]; }

    void Init(uint32_t m, uint32_t n, float dx, float dt, float speed, float damping);
    void Update(float dt);
    void Disturb(uint32_t i, uint32_t j, float magnitude);

private:
    uint32_t mNumRows;
    uint32_t mNumCols;

    uint32_t mVertexCount;
    uint32_t mTriangleCount;

    // Simulation constants we can precompute.
    float mK1;
    float mK2;
    float mK3;

    float mTimeStep;
    float mSpatialStep;

    DirectX::XMFLOAT3* mPrevSolution;
    DirectX::XMFLOAT3* mCurrSolution;
    DirectX::XMFLOAT3* mNormals;
    DirectX::XMFLOAT3* mTangentX;
};

